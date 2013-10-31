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
#pragma mark sys/*
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/errno.h>
#include <sys/types.h>
#pragma mark mach/*
#include <mach/mach.h>
#pragma mark mach-o/*
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>
#include <mach-o/ldsyms.h>
#include <mach-o/fat.h>
#pragma mark other
#include <fcntl.h>
#include <dlfcn.h>
#include <math.h>

#pragma mark -
#pragma mark Defines

#define Intel_x86_32bit_StackSetupLength 0x3
static uint8_t Intel_x86_32bit_StackSetup[Intel_x86_32bit_StackSetupLength] = {0x55, 0x89, 0xe5};

#define Intel_x86_64bit_StackSetupLength 0x4
static uint8_t Intel_x86_64bit_StackSetup[Intel_x86_64bit_StackSetupLength] = {0x55, 0x48, 0x89, 0xe5};

// need to figure out of it is necessary to setup a structure here for storing stack setup per arch

/* ARM stack?
 0xf0, 0xb5
 0x90, 0xb5
 0x80, 0xb5
 0xb0, 0xb5
 0x30, 0x40, 0x2d, 0xe9
 0xf0, 0x4f, 0x2d, 0xe9
 */

#pragma mark -
#pragma mark Declarations

void SDMSTBuildLibraryInfo(struct SDMSTLibrary *libTable, bool silent);
int SDMSTCompareTableEntries(const void *entry1, const void *entry2);
void SDMSTFindSubroutines(struct SDMSTLibrary *libTable, bool silent);
void SDMSTGenerateSortedSymbolTable(struct SDMSTLibrary *libTable, bool silent);
struct SDMSTBinary* SDMSTLoadBinaryFromFile(void* handle);
void SDMSTBinaryRelease(struct SDMSTBinary *binary);
uintptr_t* SDMSTGetCurrentArchFromBinary(struct SDMSTBinary *binary);
bool SMDSTSymbolDemangleAndCompare(char *symFromTable, char *symbolName);
SDMSTFunctionCall SDMSTSymbolLookup(struct SDMSTLibrary *libTable, char *symbolName);

#pragma mark -
#pragma mark Functions

void SDMSTBuildLibraryInfo(struct SDMSTLibrary *libTable, bool silent) {
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
			libTable->dependency = (struct SDMSTDependency *)calloc(0x1, sizeof(struct SDMSTDependency));
			libTable->dependencyCount = 0x0;
			for (uint32_t i = 0x0; i < libHeader->ncmds; i++) {
				switch (loadCmd->cmd) {
					case LC_SYMTAB: {
						libTable->libInfo->symtabCommands = realloc(libTable->libInfo->symtabCommands, (libTable->libInfo->symtabCount+0x1)*sizeof(struct symtab_command));
						libTable->libInfo->symtabCommands[libTable->libInfo->symtabCount] = *(struct symtab_command *)loadCmd;
						libTable->libInfo->symtabCount++;
						break;
					}
					case LC_SEGMENT:
					case LC_SEGMENT_64: {
						struct SDMSTSegmentEntry *seg = (struct SDMSTSegmentEntry *)loadCmd;
						if ((libTable->libInfo->textSeg == NULL) && !strncmp(SEG_TEXT,seg->segname,sizeof(seg->segname))) {
							libTable->libInfo->textSeg = (struct SDMSTSegmentEntry *)seg;
						} else if ((libTable->libInfo->linkSeg == NULL) && !strncmp(SEG_LINKEDIT,seg->segname,sizeof(seg->segname))) {
							libTable->libInfo->linkSeg = (struct SDMSTSegmentEntry *)seg;
						}
						break;
					}
					case LC_LOAD_DYLIB: {
						struct dylib_command *linkedLibrary = (struct dylib_command *)loadCmd;
						if (loadCmd+linkedLibrary->dylib.name.offset) {
							SDMPrint(silent,PrintCode_OK,"Found Dependency: %s",(char*)loadCmd+linkedLibrary->dylib.name.offset);
							libTable->dependency = (struct SDMSTDependency *)realloc(libTable->dependency, sizeof(struct SDMSTDependency)*(libTable->dependencyCount+0x1));
							libTable->dependency[libTable->dependencyCount].loadCmd = (uintptr_t)loadCmd;
							libTable->dependency[libTable->dependencyCount].dyl = *linkedLibrary;
							libTable->dependencyCount++;
						}
						break;
					}
					default: {
						break;
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

void SDMSTFindSubroutines(struct SDMSTLibrary *libTable, bool silent) {
	SDMPrint(silent,PrintCode_TRY,"Looking for subroutines...");
	if (libTable->libInfo->arch.type == CPU_TYPE_X86_64 || libTable->libInfo->arch.type == CPU_TYPE_I386) {
		libTable->subroutine = (struct SDMSTSubroutine *)calloc(0x1, sizeof(struct SDMSTSubroutine));
		libTable->subroutineCount = 0x0;
		uint32_t textSections = 0x0;
		if (libTable->libInfo->is64bit) {
			textSections = ((struct segment_command_64*)(libTable->libInfo->textSeg))->nsects;
		} else {
			textSections = ((struct segment_command *)(libTable->libInfo->textSeg))->nsects;
		}
		uintptr_t textSectionOffset = (uintptr_t)(libTable->libInfo->textSeg);
		if (libTable->libInfo->is64bit) {
			textSectionOffset += sizeof(struct segment_command_64);
		} else {
			textSectionOffset += sizeof(struct segment_command);
		}
		uint32_t flags = 0x0;
		uint64_t size = 0x0;
		uint64_t address = 0x0;
		for (uint32_t i = 0x0; i < textSections; i++) {
			if (libTable->libInfo->is64bit) {
				flags = ((struct section_64 *)(textSectionOffset))->flags;
				size = ((struct section_64 *)(textSectionOffset))->size;
				address = ((struct section_64 *)(textSectionOffset))->addr + _dyld_get_image_vmaddr_slide(libTable->vmIndex);
			} else {
				flags = ((struct section *)(textSectionOffset))->flags;
				size = ((struct section *)(textSectionOffset))->size;
				address = ((struct section *)(textSectionOffset))->addr + _dyld_get_image_vmaddr_slide(libTable->vmIndex);
			}
			Dl_info info;
			uint32_t loaded = dladdr((void*)(address), &info);
 			if (loaded != 0x0) {
				if (((flags & S_REGULAR)==0x0) && ((flags & S_ATTR_PURE_INSTRUCTIONS) || (flags & S_ATTR_SOME_INSTRUCTIONS))) {
					uint64_t offset = 0x0;
					bool isIntel64bitArch = (libTable->libInfo->is64bit && libTable->libInfo->arch.type == CPU_TYPE_X86_64);
					while (offset < (size - (isIntel64bitArch ? Intel_x86_64bit_StackSetupLength : Intel_x86_32bit_StackSetupLength))) {
						uint32_t result = 0x0;
						if (isIntel64bitArch) {
							result = memcmp((void*)(address+offset), &(Intel_x86_64bit_StackSetup[0x0]), Intel_x86_64bit_StackSetupLength);
						} else {
							result = memcmp((void*)(address+offset), &(Intel_x86_32bit_StackSetup[0x0]), Intel_x86_32bit_StackSetupLength);
						}
						if (!result) {
							char *buffer = calloc(0x1, sizeof(char)*0x400);
							libTable->subroutine = realloc(libTable->subroutine, ((libTable->subroutineCount+0x1)*sizeof(struct SDMSTSubroutine)));
							struct SDMSTSubroutine *aSubroutine = (struct SDMSTSubroutine *)calloc(0x1, sizeof(struct SDMSTSubroutine));
							aSubroutine->offset = (uintptr_t)(address+offset);
							
							unsigned int offset = (unsigned int)(aSubroutine->offset-_dyld_get_image_vmaddr_slide(libTable->libInfo->imageNumber));
							sprintf(buffer, "%x", offset);
							aSubroutine->name = calloc(0x5 + (strlen(buffer)), sizeof(char));
							sprintf(aSubroutine->name, "sub_%x", offset);
							
							aSubroutine->sectionOffset = textSectionOffset;
							
							memcpy(&(libTable->subroutine[libTable->subroutineCount]), aSubroutine, sizeof(struct SDMSTSubroutine));
							free(aSubroutine);
							free(buffer);
							libTable->subroutineCount++;
						}
						offset++;
					}
				}
			} else {
				SDMPrint(silent,PrintCode_ERR,"Image for address (%08llx) is not loaded",address);
			}
			textSectionOffset += (libTable->libInfo->is64bit ? sizeof(struct section_64) : sizeof(struct section));
		}
		SDMPrint(silent,PrintCode_OK,"Found %i subroutines",libTable->subroutineCount);
	} else {
		SDMPrint(silent,PrintCode_ERR,"Daodan only supports subroutines on Intel binaries");
	}
}

struct SDMSTRange SDMSTRangeOfSubroutine(struct SDMSTSubroutine *subroutine, struct SDMSTLibrary *libTable) {
	struct SDMSTRange range = {0x0, 0x0};
	for (uint32_t i = 0x0; i < libTable->subroutineCount; i++) {
		if (libTable->subroutine[i].offset == subroutine->offset) {
			range.offset = subroutine->offset;
			uint32_t next = i+0x1;
			if (next < libTable->subroutineCount) {
				range.length = (libTable->subroutine[next].offset - range.offset);
			} else {
				uint64_t size, address;
				if (libTable->libInfo->is64bit) {
					size = ((struct section_64 *)(subroutine->sectionOffset))->size;
					address = ((struct section_64 *)(subroutine->sectionOffset))->addr + _dyld_get_image_vmaddr_slide(libTable->vmIndex);
				} else {
					size = ((struct section *)(subroutine->sectionOffset))->size;
					address = ((struct section *)(subroutine->sectionOffset))->addr + _dyld_get_image_vmaddr_slide(libTable->vmIndex);
				}
				range.length = ((address+size) - range.offset);
			}
			break;
		}
	}
	return range;
}

void SDMSTGenerateSortedSymbolTable(struct SDMSTLibrary *libTable, bool silent) {
	SDMPrint(silent,PrintCode_TRY,"Looking for symbols...");
	if (libTable->table == NULL) {
		uintptr_t symbolAddress = 0x0;
		libTable->table = (struct SDMSTMachOSymbol *)calloc(0x1, sizeof(struct SDMSTMachOSymbol));
		libTable->symbolCount = 0x0;
		if (libTable->libInfo == NULL)
			SDMSTBuildLibraryInfo(libTable, silent);
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
				if (aSymbol) {
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
	SDMPrint(silent,PrintCode_OK,"Found %i symbols",libTable->symbolCount);
}

struct SDMSTBinary* SDMSTLoadBinaryFromFile(void* handle) {
	struct SDMSTBinary *binary = (struct SDMSTBinary *)calloc(0x1, sizeof(struct SDMSTBinary));
	binary->arch = (uintptr_t *)calloc(0x1, sizeof(uintptr_t));
	binary->archCount = 0x0;
	struct fat_header *header = handle;
	for (uint32_t i = 0x0; i < header->nfat_arch; i++) {
		struct fat_arch *arch = (handle+(i*sizeof(struct fat_arch)));
		binary->arch = realloc(binary->arch, (binary->archCount+0x1)*sizeof(uintptr_t));
		binary->arch[i] = arch->offset;
		binary->archCount++;
	}
	return binary;
}

void SDMSTBinaryRelease(struct SDMSTBinary *binary) {
	free(binary->arch);
	free(binary);
}

uintptr_t* SDMSTGetCurrentArchFromBinary(struct SDMSTBinary *binary) {
	uintptr_t *offset = NULL;
	if (binary) {
		cpu_type_t cpuType;
		size_t cpuTypeSize;
		int mib[CTL_MAXNAME];
		size_t mibLen;
		pid_t pid = getpid();
		mibLen  = CTL_MAXNAME;
		int err = sysctlnametomib("sysctl.proc_cputype", mib, &mibLen);
		if (!err) {
			mib[mibLen] = pid;
			mibLen += 0x1;
			cpuTypeSize = sizeof(cpuType);
			err = sysctl(mib, (u_int)mibLen, &cpuType, &cpuTypeSize, 0x0, 0x0);
			if (!err) {
				for (uint32_t i = 0x0; i < binary->archCount; i++) {
					struct mach_header *header = (struct mach_header *)(binary->arch[i]);
					if (header->cputype == cpuType) {
						offset = (uintptr_t *)header;
					}
				}
			}
		}

	}
	return offset;
}

struct SDMSTLibrary* SDMSTLoadLibrary(char *path, uint32_t index, bool silent) {
	struct SDMSTLibrary *table = (struct SDMSTLibrary *)calloc(0x1, sizeof(struct SDMSTLibrary));
	bool inMemory = FALSE;
	char *imagePath;
	void* handle = NULL;
	uint32_t count = _dyld_image_count();
	for (uint32_t index = 0x0; index < count; index++) {
		if (_dyld_get_image_name(index)) {
			imagePath = calloc(0x1, strlen(_dyld_get_image_name(index))+1);
			strcpy(imagePath, _dyld_get_image_name(index));
			if (strcmp(path, imagePath) == 0x0) {
				inMemory = TRUE;
				handle = (void*)_dyld_get_image_vmaddr_slide(index);
				SDMPrint(silent, PrintCode_OK,"Found Mach-O: %s",path);
				free(imagePath);
				break;
			}
			free(imagePath);
		}
	}
	if (!inMemory) {
		handle = dlopen(path, RTLD_LOCAL);
		if (!handle) {
			SDMPrint(silent,PrintCode_ERR,"Code: %s Unable to load library at path: %s", dlerror(), path);
			SDMPrint(silent,PrintCode_TRY,"Attempting to manually load and map...");
			table->couldLoad = FALSE;
			struct stat fs;
			stat(path, &fs);
			int fd = open(path, O_RDONLY);
			uint32_t header = 0xdeadbeef;
			read(fd, &header, sizeof(uint32_t));
			uint32_t size = (uint32_t)fs.st_size;
			uint32_t offset = 0x0;
			handle = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, offset);
			close(fd);
			table->librarySize = 0x0;
		}
	} else {
		table->couldLoad = TRUE;
		table->librarySize = 0x0;
	}
	if (handle || inMemory) {
		uint32_t header = (uint32_t)(handle);
		if (header == FAT_MAGIC || header == FAT_CIGAM) {
			struct SDMSTBinary *binary = SDMSTLoadBinaryFromFile(handle);
			handle = SDMSTGetCurrentArchFromBinary(binary);
			SDMSTBinaryRelease(binary);
		}
		table->vmIndex = index;
		table->libraryPath = path;
		table->libraryHandle = handle;
		table->libInfo = NULL;
		table->table = NULL;
		table->symbolCount = 0x0;
		SDMSTBuildLibraryInfo(table, silent);
		SDMSTGenerateSortedSymbolTable(table, silent);
		SDMSTFindSubroutines(table, silent);
	} else {
		table->couldLoad = FALSE;
		SDMPrint(silent,PrintCode_ERR,"Could not load MachO");
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

SDMSTFunctionCall SDMSTSymbolLookup(struct SDMSTLibrary *libTable, char *symbolName) {
	SDMSTFunctionCall symbolAddress = NULL;
	for (uint32_t i = 0x0; i < libTable->symbolCount; i++)
		if (SMDSTSymbolDemangleAndCompare(libTable->table[i].name, symbolName)) {
			symbolAddress = (SDMSTFunctionCall)libTable->table[i].offset;
			break;
		}
	return symbolAddress;
}

struct SDMSTFunction* SDMSTCreateFunction(struct SDMSTLibrary *libTable, char *name) {
	struct SDMSTFunction *function = (struct SDMSTFunction*)calloc(0x1, sizeof(struct SDMSTFunction));
	function->name = name;
	function->offset = SDMSTSymbolLookup(libTable, name);
	return function;
}

void SDMSTFunctionRelease(struct SDMSTFunction *function) {
	free(function);
}

void SDMSTLibraryRelease(struct SDMSTLibrary *libTable) {
	free(libTable->libInfo);
	for (uint32_t i = 0x0; i < libTable->symbolCount; i++) {
		if (libTable->table[i].isStub)
			free(libTable->table[i].name);
	}
	free(libTable->table);
	for (uint32_t i = 0x0; i < libTable->subroutineCount; i++) {
		free(libTable->subroutine[i].name);
	}
	free(libTable->subroutine);
	if (libTable->couldLoad)
		dlclose(libTable->libraryHandle);
	else
		munmap(libTable->libraryHandle, (size_t)libTable->librarySize);
	free(libTable);
}

#endif