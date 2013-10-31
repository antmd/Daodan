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
	char *storage = calloc(0x1, sizeof(char)*(strlen(path)+0x11));
	sprintf(storage, "%s/Library/Daodan",path);
	return storage;
}

char *SDMDaodanInternalAppStoreDirectory() {
	char *storage = SDMDaodanInternalStoreDirectory();
	CFStringRef bundleId = CFBundleGetIdentifier(CFBundleGetMainBundle());
	char *bundle = SDMCFStringGetString(bundleId);
	storage = realloc(storage, sizeof(char)*(strlen(storage)+strlen(bundle)+0x2));
	sprintf(storage,"%s/",bundle);
	free(bundle);
	return storage;
}

char* SDMDaodanStorePath() {
	char *storage = SDMDaodanInternalAppStoreDirectory();
	char *launchTime = SDMGetCurrentDateString();
	storage = realloc(storage, sizeof(char)*(strlen(storage)+0x2+strlen(launchTime)));
	sprintf(storage,"%s/",launchTime);
	free(launchTime);
	return storage;
}


void SDMDaodanCheckStorePath() {
	char *daodanStore = SDMDaodanInternalStoreDirectory();
	makeNewFolderAt(daodanStore, 0x2bc);
	free(daodanStore);
	
	char *appStore = SDMDaodanInternalAppStoreDirectory();
	makeNewFolderAt(appStore, 0x2bc);
	free(appStore);
	
	char *newDump = SDMDaodanStorePath();
	makeNewFolderAt(newDump, 0x2bc);
	free(newDump);
}

void SDMDaodanWriteSubroutine(struct SDMSTSubroutine *subroutine, FILE *file) {
	
}

void SDMDaodanWriteDumpForLibrary(char *dumpPath, struct SDMSTLibrary *libTable) {
	FILE *file = fopen(dumpPath, "rw");
	
	for (uint32_t i = 0x0; i < libTable->symbolCount; i++) {
		
	}
	
	for (uint32_t i = 0x0; i < libTable->dependencyCount; i++) {
		
	}
	
	for (uint32_t i = 0x0; i < libTable->subroutineCount; i++) {
		SDMDaodanWriteSubroutine(&(libTable->subroutine[i]), file);
	}
	
	fclose(file);
}

void SDMDaodanWriteDumpForImage(char *dumpPath, char *imagePath, bool skipDependencies) {
	uint32_t index;
	for (index = 0x0; index < _dyld_image_count(); index++) {
		if (strcmp(imagePath, _dyld_get_image_name(index)))
			break;
	}
	struct SDMSTLibrary *dumpLibrary = SDMSTLoadLibrary(imagePath, index, TRUE);
	if (!skipDependencies) {
		SDMDaodanWriteDump(dumpLibrary);
	} else {
		SDMDaodanWriteDumpForLibrary(dumpPath, dumpLibrary);
	}
}

void SDMDaodanWriteDump(struct SDMSTLibrary *libTable) {
	SDMDaodanCheckStorePath();
	char *linkStore = SDMDaodanStorePath();
	SDMDaodanWriteDumpForLibrary(linkStore, libTable);
	/*
	for (uint32_t i = 0x0; i < libTable->dependencyCount; i++) {
		char *imagePath = (char*)(libTable->dependency[i].loadCmd+(libTable->dependency[i].dyl.dylib.name.offset));
		SDMDaodanWriteDumpForImage(linkStore, imagePath, TRUE);
	}
	*/
}

#endif
