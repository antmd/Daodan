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
#include "disasm.h"
#include "Daodan.h"

#define FWRITE_STRING_TO_FILE(a,b) fwrite(a, sizeof(char), strlen(a), b)

char* SDMGetCurrentDateString() {
	time_t epoch;
	time(&epoch);
	char *secstr = calloc(0x1, sizeof(char)*0x80);
	strftime(secstr, 0x80, "%S", (struct tm*)&epoch);
	return secstr;
}

char* SDMCFStringGetString(CFStringRef str) {
	char *cstr = NULL;
	if (str) {
		cstr = calloc(0x1, sizeof(char)*(CFStringGetLength(str)+0x1));
		CFStringGetCString(str, cstr, CFStringGetLength(str)+0x1, CFStringGetFastestEncoding(str));
	}
	return cstr;
}

char* SDMDaodanInternalStoreDirectory() {
	char *path = getenv("HOME");
	char *storage = calloc(0x1, sizeof(char)*(strlen(path)+0x11+0x15));
	sprintf(storage, "%s/Library/Application Support/Daodan",path);
	return storage;
}

char* SDMDaodanFindTargetName(char *targetPath) {
	CFStringRef targetExecutablePath = CFStringCreateWithCString(kCFAllocatorDefault, targetPath, kCFStringEncodingUTF8);
	CFURLRef executableURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, targetExecutablePath, kCFURLPOSIXPathStyle, false);
	CFRelease(targetExecutablePath);
	CFStringRef targetIdentifier = CFURLCopyLastPathComponent(executableURL);
	char *targetName = SDMCFStringGetString(targetIdentifier);
	CFRelease(targetIdentifier);
	CFRelease(executableURL);
	return targetName;
}

char* SDMDaodanFindTargetBundleIdentifier(char *targetPath) {
	CFStringRef targetExecutablePath = CFStringCreateWithCString(kCFAllocatorDefault, targetPath, kCFStringEncodingUTF8);
	CFURLRef executableURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, targetExecutablePath, kCFURLPOSIXPathStyle, false);
	CFRelease(targetExecutablePath);
	CFURLRef macOSURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, executableURL);
	CFURLRef contentsURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, macOSURL);
	CFRelease(macOSURL);
	CFURLRef appURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, contentsURL);
	CFRelease(contentsURL);
	CFArrayRef bundles = CFBundleCreateBundlesFromDirectory(kCFAllocatorDefault, appURL, NULL);
	CFRelease(appURL);
	CFStringRef targetIdentifier = NULL;
	for (uint32_t i = 0x0; i < CFArrayGetCount(bundles); i++) {
		CFBundleRef bundleItem = (CFBundleRef)CFArrayGetValueAtIndex(bundles, i);
		targetIdentifier = CFBundleGetIdentifier(bundleItem);
		if (targetIdentifier) {
			break;
		}
	}
	CFRelease(bundles);
	char *bundle;
	if (!targetIdentifier) {
		CFStringRef executableIdentifier = CFURLCopyLastPathComponent(executableURL);
		bundle = SDMCFStringGetString(executableIdentifier);
		CFRelease(executableIdentifier);
	} else {
		bundle = SDMCFStringGetString(targetIdentifier);
	}
	CFRelease(executableURL);
	return bundle;
}

char *SDMDaodanInternalAppStoreDirectory(char *targetPath) {
	char *storage = SDMDaodanInternalStoreDirectory();
	char *bundle = SDMDaodanFindTargetBundleIdentifier(targetPath);
	storage = realloc(storage, sizeof(char)*(strlen(storage)+strlen(bundle)+0x2));
	sprintf(storage,"%s/%s/",storage,bundle);
	free(bundle);
	return storage;
}

char* SDMDaodanStorePath(char *targetPath) {
	char *storage = SDMDaodanInternalAppStoreDirectory(targetPath);
	char *launchTime = SDMGetCurrentDateString();
	storage = realloc(storage, sizeof(char)*(strlen(storage)+0x1+strlen(launchTime)));
	sprintf(storage,"%s%s/",storage,launchTime);
	free(launchTime);
	return storage;
}

char* SDMDaodanDumpStorePath(char *targetPath) {
	char *storage = SDMDaodanStorePath(targetPath);
	char *targetName = SDMDaodanFindTargetName(targetPath);
	storage = realloc(storage, sizeof(char)*(strlen(storage)+0x2+strlen(targetName)));
	sprintf(storage,"%s%s/",storage,targetName);
	free(targetName);
	return storage;
}


void SDMDaodanCheckStorePath(char *targetPath) {
	char *daodanStore = SDMDaodanInternalStoreDirectory();
	makeNewFolderAt(daodanStore, 0700);
	free(daodanStore);
	
	char *appStore = SDMDaodanInternalAppStoreDirectory(targetPath);
	makeNewFolderAt(appStore, 0700);
	free(appStore);
	
	char *newDump = SDMDaodanStorePath(targetPath);
	makeNewFolderAt(newDump, 0700);
	free(newDump);
	
	char *appDump = SDMDaodanDumpStorePath(targetPath);
	makeNewFolderAt(appDump, 0700);
	free(appDump);
}

void SDMDaodanWriteHeaderInDumpFile(char *header, FILE *file) {
	if (file) {
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
}

void SDMDaodanWriteSubroutine(struct SDMSTSubroutine *subroutine, CoreRange range, struct SDMDisasm *disasm, FILE *file) {
	char *subroutineDefine = calloc(0x1, sizeof(char)*(strlen(subroutine->name)+0x4));
	sprintf(subroutineDefine, "\n%s:\n",subroutine->name);
	FWRITE_STRING_TO_FILE(subroutineDefine, file);
	free(subroutineDefine);
	if (range.length) {
		SDM_disasm_setbuffer(disasm, (uintptr_t)range.offset, range.length);
		while (SDM_disasm_parse(disasm)) {
			char *line = Ptr(ud_insn_asm(&(disasm->obj)));
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
	if (file) {
		SDMDaodanWriteHeaderInDumpFile("Header Information\n",file);
		FWRITE_STRING_TO_FILE(libTable->libraryPath, file);
		FWRITE_STRING_TO_FILE("\n", file);
		
		fclose(file);
	}
	free(filePath);
}

void SDMDaodanDumpSymbolInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0xb));
	sprintf(filePath, "%sSymbols.txt",dumpPath);
	FILE *file = fopen(filePath, "w");
	if (file) {
		SDMDaodanWriteHeaderInDumpFile("Symbol Table\n",file);
		for (uint32_t i = 0x0; i < libTable->symbolCount; i++) {
			char *symbolName = (char *)(libTable->table[i].name);
			char *nameAndOffset = calloc(0x1, sizeof(char)*(strlen(symbolName)+0x100));
			uintptr_t slide;
			if (libTable->couldLoad) {
				slide = _dyld_get_image_vmaddr_slide(libTable->libInfo->imageNumber);
			} else {
				slide = (uintptr_t)(libTable->libInfo->mhOffset);
			}
			sprintf(nameAndOffset, "\t0x%016lx %s\n",((libTable->table[i].offset)-slide),symbolName);
			FWRITE_STRING_TO_FILE(nameAndOffset, file);
			free(nameAndOffset);
		}
		
		fclose(file);
	}
	free(filePath);
}

void SDMDaodanDumpLibraryInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0xd));
	sprintf(filePath, "%sLibraries.txt",dumpPath);
	FILE *file = fopen(filePath, "w");
	if (file) {
		SDMDaodanWriteHeaderInDumpFile("Linked Libraries\n",file);
		for (uint32_t i = 0x0; i < libTable->dependencyCount; i++) {
			char *path = (char *)(libTable->dependency[i].loadCmd + libTable->dependency[i].dyl.dylib.name.offset);
			uintptr_t slide;
			if (libTable->couldLoad) {
				slide = _dyld_get_image_vmaddr_slide(SDMGetIndexForLibraryPath(path));
				char *slideAndPath = calloc(0x1, sizeof(char)*(strlen(path)+0x15));
				sprintf(slideAndPath, "\t0x%016lx %s\n",slide,path);
				FWRITE_STRING_TO_FILE(slideAndPath, file);
				free(slideAndPath);
			}
		}
		
		fclose(file);
	}
	free(filePath);
}

void SDMDaodanDumpSubroutineInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0x10));
	sprintf(filePath, "%sSubroutines.txt",dumpPath);
	FILE *file = fopen(filePath, "w");
	if (file) {
		SDMDaodanWriteHeaderInDumpFile("Subroutines\n",file);
		struct SDMDisasm *disasm = SDM_disasm_init((struct mach_header *)(libTable->libInfo->mhOffset));
		for (uint32_t i = 0x0; i < libTable->subroutineCount; i++) {
			CoreRange range = SDMSTRangeOfSubroutine(&(libTable->subroutine[i]), libTable);
			SDMDaodanWriteSubroutine(&(libTable->subroutine[i]), range, disasm, file);
		}
		free(disasm);
		
		fclose(file);
	}
	free(filePath);
}

void SDMDaodanDumpObjectiveCClass(char *dumpPath, struct SDMSTObjcClass *cls) {
	if (cls->className) {
		char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x5+strlen(cls->className)+0x100));
		sprintf(filePath, "%s%s.txt",dumpPath,cls->className);
		FILE *file = fopen(filePath, "w");
		if (file) {
			SDMDaodanWriteHeaderInDumpFile("Objective-C Class\n",file);
			FWRITE_STRING_TO_FILE(cls->className,file);
			FWRITE_STRING_TO_FILE("\n", file);
			
			if (cls->ivarCount) {
				SDMDaodanWriteHeaderInDumpFile("Instance Variables\n",file);
				for (uint32_t i = 0x0; i < cls->ivarCount; i++) {
					char *ivarType = cls->ivar[i].type;
					struct SDMSTObjcType *type = SDMSTObjcDecodeType(ivarType);
					if (type->tokenCount) {
						char *ivarName;
						char *pointer = SDMSTObjcPointersForToken(&(type->token[0x0]));
						if (type->token[0x0].typeName) {
							uint64_t ivarNameLength = 0x1000;
							ivarName = calloc(0x1, sizeof(char)*(uint32_t)(ivarNameLength));
							if ((strncmp((type->token[0x0].type), "id", sizeof(char)*0x2) == 0x0) && strlen(type->token[0x0].typeName)) {
								sprintf(ivarName, "\t%s %s%s",type->token[0x0].typeName,pointer,cls->ivar[i].name);
							} else {
								if (type->token[0x0].typeName && strlen(type->token[0x0].typeName)) {
									sprintf(ivarName, "\t%s %s %s%s",type->token[0x0].type,type->token[0x0].typeName,pointer,cls->ivar[i].name);
								} else {
									sprintf(ivarName, "\t%s %s%s",type->token[0x0].type,pointer,cls->ivar[i].name);
								}
							}
							if (type->token[0x0].arrayCount) {
								sprintf(ivarName, "%s[%i]",ivarName,type->token[0x0].arrayCount);
							}
						} else {
							ivarName = calloc(0x1, sizeof(char)*(strlen(cls->ivar[i].name)+0x5+strlen(pointer)+strlen(type->token[0x0].type)));
							sprintf(ivarName, "\t%s %s%s%s",ivarName,type->token[0x0].type,pointer,cls->ivar[i].name);
							if (type->token[0x0].arrayCount) {
								sprintf(ivarName, "%s[%i]",ivarName,type->token[0x0].arrayCount);
							}
						}
						sprintf(ivarName, "%s;\n",ivarName);
						FWRITE_STRING_TO_FILE(ivarName, file);
						free(ivarName);
						free(pointer);
					}
					free(type);
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
					free(type);
				}
			}
			
			if (cls->protocolCount) {
				SDMDaodanWriteHeaderInDumpFile("Protocols\n",file);
				for (uint32_t i = 0x0; i < cls->protocolCount; i++) {
					
				}
			}
			
			fclose(file);
		}
		free(filePath);
	} else {
		SDMFormatPrint(false,PrintCode_ERR,"Could not write Objective-C to dump file: %s",dumpPath);
	}
}

void SDMDaodanDumpObjectiveCInfo(char *dumpPath, struct SDMSTLibrary *libTable) {
	if (libTable->objcInfo) {
		char *filePath = calloc(0x1, sizeof(char)*(strlen(dumpPath)+0x1+0x100));
		sprintf(filePath, "%sObjective-C/",dumpPath);
		bool result = makeNewFolderAt(filePath, 0700);
		if (result) {
			for (uint32_t i = 0x0; i < libTable->objcInfo->clsCount; i++) {
				SDMDaodanDumpObjectiveCClass(filePath, &(libTable->objcInfo->cls[i]));
			}
		}
		free(filePath);
	}
}

void SDMDaodanWriteDumpForLibrary(char *dumpPath, struct SDMSTLibrary *libTable) {
	SDMFormatPrint(false,PrintCode_TRY,"Writing dump files...");
	SDMDaodanDumpBinaryInfo(dumpPath, libTable);
	SDMDaodanDumpSymbolInfo(dumpPath, libTable);
	SDMDaodanDumpLibraryInfo(dumpPath, libTable);
	SDMDaodanDumpSubroutineInfo(dumpPath, libTable);
	SDMDaodanDumpObjectiveCInfo(dumpPath, libTable);
	SDMFormatPrint(false,PrintCode_OK,"Successfully written dump to path: %s",dumpPath);
}

void SDMDaodanWriteDumpForImage(char *dumpPath, char *imagePath, bool skipDependencies) {
	uint32_t index = SDMGetIndexForLibraryPath(imagePath);
	struct SDMSTLibrary *dumpLibrary = SDMSTLoadLibrary(imagePath, index, true);
	if (!skipDependencies) {
		SDMDaodanWriteDump(dumpLibrary);
	} else {
		SDMDaodanWriteDumpForLibrary(dumpPath, dumpLibrary);
	}
}

bool SDMDaodanHasHome() {
	return (getenv("HOME") ? true : false);
}

void SDMDaodanWriteDump(struct SDMSTLibrary *libTable) {
	bool canWriteDump = SDMDaodanHasHome();
	if (canWriteDump) {
		SDMDaodanCheckStorePath(libTable->libraryPath);
		char *linkStore = SDMDaodanDumpStorePath(libTable->libraryPath);
		SDMDaodanWriteDumpForLibrary(linkStore, libTable);
	}
	/*
	for (uint32_t i = 0x0; i < libTable->dependencyCount; i++) {
		char *imagePath = Ptr((libTable->dependency[i].loadCmd+(libTable->dependency[i].dyl.dylib.name.offset)));
		SDMDaodanWriteDumpForImage(linkStore, imagePath, true);
	}
	*/
}

#endif
