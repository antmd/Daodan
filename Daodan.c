/*
 *  Daodan.c
 *  Daodan
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
#ifndef Daodan_Daodan_c
#define Daodan_Daodan_c

#include "Daodan.h"
#include <dlfcn.h>

/*
 
** How to implement function hooks:
 
 Add weak undefined functions to an application following the format:
 	extern void MyFunctionHook() __attribute__((weak));
 
 Add the linker flag to the application:
 	-undefined dynamic_lookup
 
 This will allow for you to define your own functions your own dylib that
 	can be loaded with the application with the following command:
 	$ export DYLD_FORCE_FLAT_NAMESPACE=1; export DYLD_INSERT_LIBRARIES=<path/to/dylib>; ./MyApplication.app/Contents/MacOS/MyApplication
 
** How to call existing functions:
 
 Call daodanLookupFunction() providing the mangled or unmangled name of the
 	desired function call. A pointer to the function will be returned.
 
** Using DYLD_INTERPOSE
 
 Using DYLD_INTERPOSE is only possible when this library is a dependent 
 	library of the application. If that is the case, then use the 
 	DYLD_INTERPOSE macro to set the names of the functions of the functions
 	to hook and the new function implementation to call instead.
 
 */

#define DYLD_INTERPOSE(_replacement,_replacee) \
__attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee \
__attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

static struct SDMMOLibrarySymbolTable *binaryTable;

void initDaodan() {
	_dyld_register_func_for_add_image(SDMAddImageHook);
	_dyld_register_func_for_remove_image(SDMRemoveImageHook);
	uint32_t result = SDMGetExecuteImage();
	if (result != 0xffffffff) {
		binaryTable = SDMSTLoadLibrary((char*)_dyld_get_image_name(result));
	} else {
		printf("[%sDaodan%s][%sERR%s] Could not find an executable binary image.\n",COLOR_BLU,COLOR_NRM,COLOR_RED,COLOR_NRM);
	}
}

void unloadDaodan() {
	printf("[%sDaodan%s][%sTRY%s] Looking for Daodan.\n",COLOR_BLU,COLOR_NRM,COLOR_YEL,COLOR_NRM);
	void* symbolAddress = NULL;
	symbolAddress = dlsym(RTLD_SELF, "_initDaodan");
	Dl_info libInfo;
	if (dladdr(symbolAddress, &libInfo) == 0) {
		printf("[%sDaodan%s][%sOK!%s] Found Daodan.\n",COLOR_BLU,COLOR_NRM,COLOR_GRN,COLOR_NRM);
		void* daodanHandle = dlopen(libInfo.dli_fbase, RTLD_LAZY);
		if (daodanHandle) {
			printf("[%sDaodan%s][%sOK!%s] Unloading Daodan.\n",COLOR_BLU,COLOR_NRM,COLOR_GRN,COLOR_NRM);
		} else {
			printf("[%sDaodan%s][%sERR%s] Error creating handle to Daodan.\n",COLOR_BLU,COLOR_NRM,COLOR_RED,COLOR_NRM);
		}
		SDMSTLibraryRelease(binaryTable);
		dlclose(daodanHandle);
	} else {
		printf("[%sDaodan%s][%sERR%s] Could not find Daodan.\n",COLOR_BLU,COLOR_NRM,COLOR_RED,COLOR_NRM);
	}
}

uintptr_t daodanLookupFunction(char *name) {
	printf("[%sDaodan%s][%sTRY%s] Looking up function with name: %s\n",COLOR_BLU,COLOR_NRM,COLOR_YEL,COLOR_NRM,name);
	struct SDMSTFunction *symbol = SDMSTCreateFunction(binaryTable, name);
	if (symbol->offset) {
		printf("[%sDaodan%s][%sOK!%s] Successfully found symbol!\n",COLOR_BLU,COLOR_NRM,COLOR_GRN,COLOR_NRM);
	} else {
		printf("[%sDaodan%s][%sERR%s] Could not find symbol with name \"%s\".\n",COLOR_BLU,COLOR_NRM,COLOR_RED,COLOR_NRM,name);
	}
	return (uintptr_t)symbol->offset;
}

void daodanLoadSymbolTableForImage(uint32_t index) {
	SDMSTLibraryRelease(binaryTable);
	binaryTable = SDMSTLoadLibrary((char*)_dyld_get_image_name(index));
}

uint32_t SDMGetExecuteImage() {
	printf("[%sDaodan%s][%sTRY%s] Looking for application binary.\n",COLOR_BLU,COLOR_NRM,COLOR_YEL,COLOR_NRM);
	struct mach_header *imageHeader;
	uint32_t count = _dyld_image_count();
	bool foundBinary = FALSE;
	uint32_t index;
	for (index = 0x0; index < count; index++) {
		imageHeader = (struct mach_header *)_dyld_get_image_header(index);
		if (imageHeader->filetype == MH_EXECUTE) {
			foundBinary = TRUE;
			break;
		}
	}
	return (foundBinary ? index : 0xffffffff);
}

uint32_t SDMGetImageLocation(const struct mach_header *mh, char **path) {
	struct mach_header *imageHeader;
	uint32_t count = _dyld_image_count();
	uint32_t index;
	for (index = 0x0; index < count; index++) {
		imageHeader = (struct mach_header *)_dyld_get_image_header(index);
		if (memcmp(imageHeader, mh, sizeof(struct mach_header)) == 0x0) {
			if (_dyld_get_image_name(index)) {
				*path = calloc(0x1, strlen(_dyld_get_image_name(index))+0x1);
				strcpy(*path, _dyld_get_image_name(index));
				break;
			}
		}
	}
	return index;
}

void SDMAddImageHook(const struct mach_header* mh, intptr_t vmaddr_slide) {
	char *path = NULL;
	SDMGetImageLocation(mh, &path);
#if DEBUG
	printf("[%sDaodan%s][%sOK!%s] Load: %08lx %s\n",COLOR_BLU,COLOR_NRM,COLOR_GRN,COLOR_NRM, vmaddr_slide, path);
#else
	// this exists for debugging, do not ship in debug mode.
#endif
	free(path);
}

void SDMRemoveImageHook(const struct mach_header* mh, intptr_t vmaddr_slide) {
	char *path = NULL;
	SDMGetImageLocation(mh, &path);
#if DEBUG
	printf("[%sDaodan%s][%sOK!%s] Unloaded: %08lx %s\n",COLOR_BLU,COLOR_NRM,COLOR_GRN,COLOR_NRM, vmaddr_slide, path);
#else
	// this exists for debugging, do not ship in debug mode.
#endif
	free(path);
}

#endif