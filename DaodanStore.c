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

char* SDMGetCurrentDateString() {
	time_t epoch;
	time(&epoch);
	char *secstr = calloc(1, 128);
	strftime(secstr, 128, "%S", (struct tm*)&epoch);
	return secstr;
}

char* SDMCFStringGetString(CFStringRef str) {
	char *cstr = calloc(1, CFStringGetLength(str)+1);
	CFStringGetCString(str, cstr, CFStringGetLength(str)+1, CFStringGetFastestEncoding(str));
	return cstr;
}

char* SDMDaodanInternalStoreDirectory() {
	char *path = getenv("HOME");
	char *storage = calloc(0x1, strlen(path)+17);
	strcat(storage, path);
	strcat(storage, "/Library/Daodan/");
	return storage;
}

char *SDMDaodanInternalAppStoreDirectory() {
	char *storage = SDMDaodanInternalStoreDirectory();
	CFStringRef bundleId = CFBundleGetIdentifier(CFBundleGetMainBundle());
	char *bundle = SDMCFStringGetString(bundleId);
	storage = realloc(storage, strlen(storage)+strlen(bundle)+2);
	strcat(storage, bundle);
	strcat(storage, "/");
	free(bundle);
	return storage;
}

char* SDMDaodanStorePath() {
	char *storage = SDMDaodanInternalAppStoreDirectory();
	char *launchTime = SDMGetCurrentDateString();
	storage = realloc(storage, strlen(storage)+2+strlen(launchTime));
	strcat(storage, launchTime);
	strcat(storage, "/");
	free(launchTime);
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
}

void SDMDaodanWriteDumpForLibrary(char *dumpPath, struct SDMSTLibrary *libTable) {
	
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
	for (uint32_t i = 0x0; i < libTable->dependencyCount; i++) {
		char *imagePath = (char*)(libTable->dependency[i].loadCmd+(libTable->dependency[i].dyl.dylib.name.offset));
		SDMDaodanWriteDumpForImage(linkStore, imagePath, TRUE);
	}
}

#endif
