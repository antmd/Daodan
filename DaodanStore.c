//
//  DaodanStore.c
//  Daodan
//
//  Created by Sam Marshall on 10/31/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_DaodanStore_c
#define Daodan_DaodanStore_c

#include "DaodanStore.h"
#include <CoreFoundation/CoreFoundation.h>
#include <mach-o/dyld.h>
#include "disasm.h"
#include "Daodan.h"

char* SDMGetCurrentDateString() {
	time_t epoch;
	time(&epoch);
	char *secstr = calloc(0x1, sizeof(char)*0x80);
	strftime(secstr, 0x80, "%S", (struct tm*)&epoch);
	return secstr;
}

char* SDMCFStringGetString(CFStringRef str) {
	char *cstr = calloc(0x1, sizeof(char)*(CFStringGetLength(str)+0x1));
	CFStringGetCString(str, cstr, CFStringGetLength(str)+0x1, CFStringGetFastestEncoding(str));
	return cstr;
}

char* SDMDaodanInternalStoreDirectory() {
	char *path = getenv("HOME");
	char *storage = calloc(0x1, sizeof(char)*(strlen(path)+0x11+0x15));
	sprintf(storage, "%s/Library/Application Support/Daodan",path);
	return storage;
}

char *SDMDaodanInternalAppStoreDirectory() {
	char *storage = SDMDaodanInternalStoreDirectory();
	CFStringRef bundleId = CFBundleGetIdentifier(CFBundleGetMainBundle());
	char *bundle = SDMCFStringGetString(bundleId);
	storage = realloc(storage, sizeof(char)*(strlen(storage)+strlen(bundle)+0x2));
	sprintf(storage,"%s/%s/",storage,bundle);
	free(bundle);
	return storage;
}

char* SDMDaodanStorePath() {
	char *storage = SDMDaodanInternalAppStoreDirectory();
	char *launchTime = SDMGetCurrentDateString();
	storage = realloc(storage, sizeof(char)*(strlen(storage)+0x1+strlen(launchTime)));
	sprintf(storage,"%s%s/",storage,launchTime);
	free(launchTime);
	return storage;
}

char* SDMDaodanDumpStorePath() {
	char *storage = SDMDaodanStorePath();
	storage = realloc(storage, sizeof(char)*(strlen(storage)+0x2+strlen(getprogname())));
	sprintf(storage,"%s%s/",storage,getprogname());
	return storage;
}


void SDMDaodanCheckStorePath() {
	char *daodanStore = SDMDaodanInternalStoreDirectory();
	makeNewFolderAt(daodanStore, 0700);
	free(daodanStore);
	
	char *appStore = SDMDaodanInternalAppStoreDirectory();
	makeNewFolderAt(appStore, 0700);
	free(appStore);
	
	char *newDump = SDMDaodanStorePath();
	makeNewFolderAt(newDump, 0700);
	free(newDump);
	
	char *appDump = SDMDaodanDumpStorePath();
	makeNewFolderAt(appDump, 0700);
	free(appDump);
}

void SDMDaodanWriteHeaderInDumpFile(char *header, FILE *file) {
	FWRITE_STRING_TO_FILE("\n",file);
	for (uint32_t i = 0x0; i < 0x50; i++) {
		FWRITE_STRING_TO_FILE("=", file);
	}
	FWRITE_STRING_TO_FILE("\n",file);
	FWRITE_STRING_TO_FILE(header, file);
	for (uint32_t i = 0x0; i < 0x50; i++) {
		FWRITE_STRING_TO_FILE("=", file);
	}
	FWRITE_STRING_TO_FILE("\n",file);
}

void SDMDaodanWriteSubroutine(struct SDMSTSubroutine *subroutine, struct SDMSTRange range, struct SDMDisasm *disasm, FILE *file) {
	char *subroutineDefine = calloc(0x1, sizeof(char)*(strlen(subroutine->name)+0x4));
	sprintf(subroutineDefine, "\n%s:\n",subroutine->name);
	FWRITE_STRING_TO_FILE(subroutineDefine, file);
	free(subroutineDefine);
	if (range.length) {
		SDM_disasm_setbuffer(disasm, range.offset, range.length);
		while (SDM_disasm_parse(disasm)) {
			char *line = (char*)ud_insn_asm(&(disasm->obj));
			char *printLine = calloc(0x1, sizeof(char)*(strlen(line)+0x3));
			sprintf(printLine,"\t%s\n",line);
			FWRITE_STRING_TO_FILE(printLine, file);
			free(printLine);
		}
	}
}

void SDMDaodanDumpBinaryInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0xa));
	sprintf(filePath, "%sBinary.txt",dumpPath);
	FILE *file = fopen(filePath, "w+");

	SDMDaodanWriteHeaderInDumpFile("Header Information\n",file);
	FWRITE_STRING_TO_FILE(libTable->libraryPath, file);
	FWRITE_STRING_TO_FILE("\n", file);
	
	fclose(file);
	free(filePath);
}

void SDMDaodanDumpSymbolInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0xb));
	sprintf(filePath, "%sSymbols.txt",dumpPath);
	FILE *file = fopen(filePath, "w+");
	
	SDMDaodanWriteHeaderInDumpFile("Symbol Table\n",file);
	for (uint32_t i = 0x0; i < libTable->symbolCount; i++) {
		char *symbolName = (char *)(libTable->table[i].name);
		char *nameAndOffset = calloc(0x1, sizeof(char)*(strlen(symbolName)+0x15));
		uintptr_t slide = _dyld_get_image_vmaddr_slide(libTable->libInfo->imageNumber);
		sprintf(nameAndOffset, "\t0x%016lx %s\n",((libTable->table[i].offset)-slide),symbolName);
		FWRITE_STRING_TO_FILE(nameAndOffset, file);
		free(nameAndOffset);
	}
	
	fclose(file);
	free(filePath);
}

void SDMDaodanDumpLibraryInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0xd));
	sprintf(filePath, "%sLibraries.txt",dumpPath);
	FILE *file = fopen(filePath, "w+");
	
	SDMDaodanWriteHeaderInDumpFile("Linked Libraries\n",file);
	for (uint32_t i = 0x0; i < libTable->dependencyCount; i++) {
		char *path = (char *)(libTable->dependency[i].loadCmd + libTable->dependency[i].dyl.dylib.name.offset);
		uintptr_t slide = _dyld_get_image_vmaddr_slide(SDMGetIndexForLibraryPath(path));
		char *slideAndPath = calloc(0x1, sizeof(char)*(strlen(path)+0x15));
		sprintf(slideAndPath, "\t0x%016lx %s\n",slide,path);
		FWRITE_STRING_TO_FILE(slideAndPath, file);
		free(slideAndPath);
	}
	
	fclose(file);
	free(filePath);
}

void SDMDaodanDumpSubroutineInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0xd));
	sprintf(filePath, "%sLibraries.txt",dumpPath);
	FILE *file = fopen(filePath, "w+");
	
	SDMDaodanWriteHeaderInDumpFile("Subroutines\n",file);
	struct SDMDisasm *disasm = SDM_disasm_init((struct mach_header *)(libTable->libInfo->mhOffset));
	for (uint32_t i = 0x0; i < libTable->subroutineCount; i++) {
		struct SDMSTRange range = SDMSTRangeOfSubroutine(&(libTable->subroutine[i]), libTable);
		SDMDaodanWriteSubroutine(&(libTable->subroutine[i]), range, disasm, file);
	}
	free(disasm);
	
	fclose(file);
	free(filePath);
}

void SDMDaodanDumpObjectiveCClass(char *dumpPath, struct SDMSTObjcClass *cls) {
	if (cls->className) {
		char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x5+strlen(cls->className)));
		sprintf(filePath, "%s%s.txt",dumpPath,cls->className);
		FILE *file = fopen(filePath, "w+");
		
		SDMDaodanWriteHeaderInDumpFile("Objective-C Class\n",file);
		
		if (cls->ivarCount) {
			SDMDaodanWriteHeaderInDumpFile("Instance Variables\n",file);
			for (uint32_t i = 0x0; i < cls->ivarCount; i++) {
				char *ivarType = cls->ivar[i].type;
				struct SDMSTObjcType *type = SDMSTObjcDecodeType(ivarType);
				if (type->tokenCount) {
					char *ivarName;
					char *pointer = SDMSTObjcPointersForToken(&(type->token[0x0]));
					if (type->token[0x0].typeName) {
						ivarName = calloc(0x1, sizeof(char)*(strlen(cls->ivar[i].name)+0x5+strlen(pointer)+strlen(type->token[0x0].type)+strlen(type->token[0x0].typeName)));
						sprintf(ivarName, "\t%s %s %s%s;\n",type->token[0x0].type,type->token[0x0].typeName,pointer,cls->ivar[i].name);
					} else {
						ivarName = calloc(0x1, sizeof(char)*(strlen(cls->ivar[i].name)+0x5+strlen(pointer)+strlen(type->token[0x0].type)));
						sprintf(ivarName, "\t%s %s%s;\n",type->token[0x0].type,pointer,cls->ivar[i].name);
					}
					FWRITE_STRING_TO_FILE(ivarName, file);
					free(ivarName);
					free(pointer);
				}
			}
		}
		
		if (cls->methodCount) {
			SDMDaodanWriteHeaderInDumpFile("Methods\n",file);
			for (uint32_t i = 0x0; i < cls->methodCount; i++) {
				char *methodType = cls->method[i].type;
				struct SDMSTObjcType *type = SDMSTObjcDecodeType(methodType);
				if (type->tokenCount) {
					char *methodDescription = SDMSTObjcCreateMethodDescription(type, (cls->method[i].name));
					FWRITE_STRING_TO_FILE("\t", file);
					FWRITE_STRING_TO_FILE(methodDescription, file);
					FWRITE_STRING_TO_FILE("\n", file);
					free(methodDescription);
				}
			}
		}
		
		if (cls->protocolCount) {
			SDMDaodanWriteHeaderInDumpFile("Protocols\n",file);
			for (uint32_t i = 0x0; i < cls->protocolCount; i++) {

			}
		}
		
		fclose(file);
		free(filePath);
	}
}

void SDMDaodanDumpObjectiveCInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0xd));
	sprintf(filePath, "%sObjective-C/",dumpPath);
	makeNewFolderAt(filePath, 0700);
	for (uint32_t i = 0x0; i < libTable->objcInfo->clsCount; i++) {
		SDMDaodanDumpObjectiveCClass(filePath, &(libTable->objcInfo->cls[i]));
	}
	free(filePath);
}

void SDMDaodanWriteDumpForLibrary(char *dumpPath, struct SDMSTLibrary *libTable) {
	SDMPrint(FALSE,PrintCode_TRY,"Writing dump files...");
	SDMDaodanDumpBinaryInfo(dumpPath, libTable);
	SDMDaodanDumpSymbolInfo(dumpPath, libTable);
	SDMDaodanDumpLibraryInfo(dumpPath, libTable);
	SDMDaodanDumpSubroutineInfo(dumpPath, libTable);
	SDMDaodanDumpObjectiveCInfo(dumpPath, libTable);
	SDMPrint(FALSE,PrintCode_OK,"Successfully written dump to path: %s",dumpPath);
}

void SDMDaodanWriteDumpForImage(char *dumpPath, char *imagePath, bool skipDependencies) {
	uint32_t index = SDMGetIndexForLibraryPath(imagePath);
	struct SDMSTLibrary *dumpLibrary = SDMSTLoadLibrary(imagePath, index, TRUE);
	if (!skipDependencies) {
		SDMDaodanWriteDump(dumpLibrary);
	} else {
		SDMDaodanWriteDumpForLibrary(dumpPath, dumpLibrary);
	}
}

void SDMDaodanWriteDump(struct SDMSTLibrary *libTable) {
	SDMDaodanCheckStorePath();
	char *linkStore = SDMDaodanDumpStorePath();
	SDMDaodanWriteDumpForLibrary(linkStore, libTable);
	/*
	for (uint32_t i = 0x0; i < libTable->dependencyCount; i++) {
		char *imagePath = (char*)(libTable->dependency[i].loadCmd+(libTable->dependency[i].dyl.dylib.name.offset));
		SDMDaodanWriteDumpForImage(linkStore, imagePath, TRUE);
	}
	*/
}

#endif
