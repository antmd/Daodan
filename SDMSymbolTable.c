/*
 *  SDMSymbolTable.c
 *  SDMSymbolTable
 *
 *  Copyright (c) 2013, Sam Marshall
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
 *  	This product includes software developed by the Sam Marshall.
 *  4. Neither the name of the Sam Marshall nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY Sam Marshall ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Sam Marshall BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _SDMSYMBOLTABLE_C_
#define _SDMSYMBOLTABLE_C_

#pragma mark -
#pragma mark Includes
#include "SDMSymbolTable.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>
#include <mach-o/ldsyms.h>
#include <math.h>

#pragma mark -
#pragma mark Declarations

void SDMSTBuildLibraryInfo(SDMMOLibrarySymbolTable *libTable);
int SDMSTCompareTableEntries(const void *entry1, const void *entry2);
void SDMSTGenerateSortedSymbolTable(struct SDMMOLibrarySymbolTable *libTable);
bool SMDSTSymbolDemangleAndCompare(char *symFromTable, char *symbolName);
SDMSTFunctionCall SDMSTSymbolLookup(struct SDMMOLibrarySymbolTable *libTable, char *symbolName);

extern void* makeDynamicCallWithIntList(uint32_t argc, void* argv, void* functionPointer);

#pragma mark -
#pragma mark Functions

void SDMSTBuildLibraryInfo(SDMMOLibrarySymbolTable *libTable) {
	if (libTable->libInfo == NULL) {
		libTable->libInfo = (struct SDMSTLibraryTableInfo *)calloc(0x1, sizeof(struct SDMSTLibraryTableInfo));
		const struct mach_header *imageHeader;
		if (libTable->couldLoad) {
			uint32_t count = _dyld_image_count();
			for (uint32_t i = 0x0; i < count; i++) {
				if (strcmp(_dyld_get_image_name(i), libTable->libraryPath) == 0x0) {
					libTable->libInfo->imageNumber = i;
					break;
				}
			}
			imageHeader = _dyld_get_image_header(libTable->libInfo->imageNumber);
		} else {
			imageHeader = (struct mach_header *)libTable->libraryHandle;
		}
		libTable->libInfo->headerMagic = imageHeader->magic;
		libTable->libInfo->arch = (struct SDMSTLibraryArchitecture){imageHeader->cputype, imageHeader->cpusubtype};
		libTable->libInfo->is64bit = ((imageHeader->cputype == CPU_TYPE_X86_64 || imageHeader->cputype == CPU_TYPE_POWERPC64) && (libTable->libInfo->headerMagic == MH_MAGIC_64 || libTable->libInfo->headerMagic == MH_CIGAM_64) ? true : false);
		libTable->libInfo->mhOffset = (uintptr_t*)imageHeader;
	}
	struct mach_header *libHeader = (struct mach_header *)((char*)libTable->libInfo->mhOffset);
	if (libTable->libInfo->headerMagic == libHeader->magic) {
		if (libTable->libInfo->symtabCommands == NULL) {
			struct load_command *loadCmd = (struct load_command *)((char*)libTable->libInfo->mhOffset + (libTable->libInfo->is64bit ? sizeof(struct mach_header_64) : sizeof(struct mach_header)));
			libTable->libInfo->symtabCommands = (struct symtab_command *)calloc(0x1, sizeof(struct symtab_command));
			libTable->libInfo->symtabCount = 0x0;
			for (uint32_t i = 0x0; i < libHeader->ncmds; i++) {
				if (loadCmd->cmd == LC_SYMTAB) {
					libTable->libInfo->symtabCommands = realloc(libTable->libInfo->symtabCommands, (libTable->libInfo->symtabCount+0x1)*sizeof(struct symtab_command));
					libTable->libInfo->symtabCommands[libTable->libInfo->symtabCount] = *(struct symtab_command *)loadCmd;
					libTable->libInfo->symtabCount++;
				}
				if (loadCmd->cmd == (libTable->libInfo->is64bit ? LC_SEGMENT_64 : LC_SEGMENT)) {
					struct SDMSTSegmentEntry *seg = (struct SDMSTSegmentEntry *)loadCmd;
					if ((libTable->libInfo->textSeg == NULL) && !strncmp(SEG_TEXT,seg->segname,sizeof(seg->segname))) {
						libTable->libInfo->textSeg = (struct SDMSTSegmentEntry *)seg;
					} else if ((libTable->libInfo->linkSeg == NULL) && !strncmp(SEG_LINKEDIT,seg->segname,sizeof(seg->segname))) {
						libTable->libInfo->linkSeg = (struct SDMSTSegmentEntry *)seg;
					}
				}
				if (loadCmd->cmd == LC_LOAD_DYLIB) {
					struct dylib_command *linkedLibrary = (struct dylib_command *)loadCmd;
					if (loadCmd+linkedLibrary->dylib.name.offset) {
						printf("%s\n",(char*)loadCmd+linkedLibrary->dylib.name.offset);
					}
				}
				loadCmd = (struct load_command *)((char*)loadCmd + loadCmd->cmdsize);
			}
		}
	}
}

int SDMSTCompareTableEntries(const void *entry1, const void *entry2) {
	if (((struct SDMSTMachOSymbol *)entry1)->offset < ((struct SDMSTMachOSymbol *)entry2)->offset) return 0xffffffff;
	if (((struct SDMSTMachOSymbol *)entry1)->offset == ((struct SDMSTMachOSymbol *)entry2)->offset) return 0x0;
	if (((struct SDMSTMachOSymbol *)entry1)->offset > ((struct SDMSTMachOSymbol *)entry2)->offset) return 0x1;
	return 0xdeadbeef;
}

void SDMSTGenerateSortedSymbolTable(struct SDMMOLibrarySymbolTable *libTable) {
	if (libTable->table == NULL) {
		uintptr_t symbolAddress = 0x0;
		libTable->table = (struct SDMSTMachOSymbol *)calloc(0x1, sizeof(struct SDMSTMachOSymbol));
		libTable->symbolCount = 0x0;
		if (libTable->libInfo == NULL)
			SDMSTBuildLibraryInfo(libTable);
		for (uint32_t i = 0x0; i < libTable->libInfo->symtabCount; i++) {
			struct symtab_command *cmd = (struct symtab_command *)(&(libTable->libInfo->symtabCommands[i]));
			uint64_t fslide = 0x0;
			if (libTable->libInfo->is64bit) {
				struct SDMSTSeg64Data *textData = (struct SDMSTSeg64Data *)((char*)libTable->libInfo->textSeg + sizeof(struct SDMSTSegmentEntry));
				struct SDMSTSeg64Data *linkData = (struct SDMSTSeg64Data *)((char*)libTable->libInfo->linkSeg + sizeof(struct SDMSTSegmentEntry));
				fslide = (uint64_t)(linkData->vmaddr - textData->vmaddr) - linkData->fileoff;
			} else {
				struct SDMSTSeg32Data *textData = (struct SDMSTSeg32Data *)((char*)libTable->libInfo->textSeg + sizeof(struct SDMSTSegmentEntry));
				struct SDMSTSeg32Data *linkData = (struct SDMSTSeg32Data *)((char*)libTable->libInfo->linkSeg + sizeof(struct SDMSTSegmentEntry));
				fslide = (uint64_t)(linkData->vmaddr - textData->vmaddr) - linkData->fileoff;
			}
			struct SDMSTSymbolTableListEntry *entry = (struct SDMSTSymbolTableListEntry *)((char*)libTable->libInfo->mhOffset + cmd->symoff + fslide);
			for (uint32_t j = 0x0; j < cmd->nsyms; j++) {
				if (!(entry->n_type & N_STAB) && ((entry->n_type & N_TYPE) == N_SECT)) {
					char *strTable = (char*)libTable->libInfo->mhOffset + cmd->stroff + fslide;
					if (libTable->libInfo->is64bit) {
						uint64_t *n_value = (uint64_t*)((char*)entry + sizeof(struct SDMSTSymbolTableListEntry));
						symbolAddress = (uintptr_t)*n_value;
					} else {
						uint32_t *n_value = (uint32_t*)((char*)entry + sizeof(struct SDMSTSymbolTableListEntry));
						symbolAddress = (uintptr_t)*n_value;
					}
					libTable->table = realloc(libTable->table, sizeof(struct SDMSTMachOSymbol)*(libTable->symbolCount+0x1));
					struct SDMSTMachOSymbol *aSymbol = (struct SDMSTMachOSymbol *)calloc(0x1, sizeof(struct SDMSTMachOSymbol));
					aSymbol->tableNumber = i;
					aSymbol->symbolNumber = j;
					aSymbol->offset = (uintptr_t)symbolAddress + (libTable->couldLoad ? _dyld_get_image_vmaddr_slide(libTable->libInfo->imageNumber) : 0x0);
					if (entry->n_un.n_strx && entry->n_un.n_strx < cmd->strsize) {
						aSymbol->name = ((char *)strTable + entry->n_un.n_strx);
						aSymbol->isStub = false;
					} else {
						aSymbol->name = calloc(0xe + ((libTable->symbolCount==0x0) ? 0x1 : (uint32_t)log10(libTable->symbolCount) + 0x1), sizeof(char));
						sprintf(aSymbol->name, "__sdmst_stub_%i", libTable->symbolCount);
						aSymbol->isStub = true;
					}
					memcpy(&libTable->table[libTable->symbolCount], aSymbol, sizeof(struct SDMSTMachOSymbol));
					free(aSymbol);
					libTable->symbolCount++;
				}
				entry = (struct SDMSTSymbolTableListEntry *)((char*)entry + (sizeof(struct SDMSTSymbolTableListEntry) + (libTable->libInfo->is64bit ? sizeof(uint64_t) : sizeof(uint32_t))));
			}
		}
		qsort(libTable->table, libTable->symbolCount, sizeof(struct SDMSTMachOSymbol), SDMSTCompareTableEntries);
	}
}

struct SDMMOLibrarySymbolTable* SDMSTLoadLibrary(char *path) {
	struct SDMMOLibrarySymbolTable *table = (struct SDMMOLibrarySymbolTable *)calloc(0x1, sizeof(struct SDMMOLibrarySymbolTable));
	bool inMemory = FALSE;
	char *imagePath;
	uint32_t count = _dyld_image_count();
	for (uint32_t index = 0x0; index < count; index++) {
		if (_dyld_get_image_name(index)) {
			imagePath = calloc(0x1, strlen(_dyld_get_image_name(index))+1);
			strcpy(imagePath, _dyld_get_image_name(index));
			if (strcmp(path, imagePath) == 0x0) {
				inMemory = TRUE;
				printf("[%sDaodan%s][%sOK!%s] Found Mach-O: %s\n",COLOR_BLU,COLOR_NRM,COLOR_GRN,COLOR_NRM,path);
				break;
			}
		}
	}
	void* handle = NULL;
	if (!inMemory) {
		handle = dlopen(path, RTLD_LOCAL);
		if (!handle) {
			printf("[%sDaodan%s][%sERR%s] Code: %s Unable to load library at path: %s\n", COLOR_BLU,COLOR_NRM,COLOR_RED,COLOR_NRM, dlerror(), path);
			printf("[%sDaodan%s][%sTRY%s] Attempting to manually load and map...\n",COLOR_BLU,COLOR_NRM,COLOR_YEL,COLOR_NRM);
			table->couldLoad = FALSE;
			struct stat fs;
			stat(path, &fs);
			int fd = open(path, O_RDONLY);
			uint32_t header = 0xdeadbeef;
			read(fd, &header, sizeof(uint32_t));
			uint32_t size = (uint32_t)fs.st_size;
			uint32_t offset = 0x0;
			if (header == 0xbebafeca) {
				size -= 0x1000;
				offset += 0x1000;
			}
			handle = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, offset);
			close(fd);
			table->librarySize = 0x0;
		}
	} else {
		table->couldLoad = TRUE;
		table->librarySize = 0x0;
	}
	if (handle) {
		table->libraryPath = path;
		table->libraryHandle = handle;
		table->libInfo = NULL;
		table->table = NULL;
		table->symbolCount = 0x0;
		SDMSTBuildLibraryInfo(table);
		SDMSTGenerateSortedSymbolTable(table);
	} else {
		
	}
	return table;
}

bool SMDSTSymbolDemangleAndCompare(char *symFromTable, char *symbolName) {
	bool matchesName = false;
	if (symFromTable && symbolName) {
		uint32_t tabSymLength = (uint32_t)strlen(symFromTable);
		uint32_t symLength = (uint32_t)strlen(symbolName);
		if (symLength <= tabSymLength) {
			char *offset = strstr(symFromTable, symbolName);
			if (offset) {
				uint32_t originOffset = (uint32_t)(offset - symFromTable);
				if (tabSymLength-originOffset == symLength)
					matchesName = (strcmp(&symFromTable[originOffset], symbolName) == 0x0);
			}
		}
	}
	return matchesName;
}

SDMSTFunctionCall SDMSTSymbolLookup(struct SDMMOLibrarySymbolTable *libTable, char *symbolName) {
	SDMSTFunctionCall symbolAddress = NULL;
	for (uint32_t i = 0x0; i < libTable->symbolCount; i++)
		if (SMDSTSymbolDemangleAndCompare(libTable->table[i].name, symbolName)) {
			symbolAddress = (SDMSTFunctionCall)libTable->table[i].offset;
			break;
		}
	return symbolAddress;
}

struct SDMSTFunction* SDMSTCreateFunction(struct SDMMOLibrarySymbolTable *libTable, char *name) {
	struct SDMSTFunction *function = (struct SDMSTFunction*)calloc(0x1, sizeof(struct SDMSTFunction));
	function->name = name;
	function->offset = SDMSTSymbolLookup(libTable, name);
	return function;
}

void SDMSTFunctionRelease(struct SDMSTFunction *function) {
	free(function);
}

void SDMSTLibraryRelease(struct SDMMOLibrarySymbolTable *libTable) {
	free(libTable->libInfo);
	for (uint32_t i = 0x0; i < libTable->symbolCount; i++) {
		if (libTable->table[i].isStub)
			free(libTable->table[i].name);
	}
	free(libTable->table);
	if (libTable->couldLoad)
		dlclose(libTable->libraryHandle);
	else
		munmap(libTable->libraryHandle, (size_t)libTable->librarySize);
	free(libTable);
}

#endif