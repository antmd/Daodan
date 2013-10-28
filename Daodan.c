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

#pragma mark -
#pragma mark Includes
#include "Daodan.h"
#include <dlfcn.h>
#include <security/mac.h>
#include <mach-o/dyld.h>

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

/***************************************************/

// This little section of code is to ensure we are running even if the sandbox is broken

int DAODAN__mac_execve(char *fname, char **argv, char **envv, mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_execve(%s, %p, %p, %p)",fname,argv,envv,_label);
#endif
	return 0x0;
}

int DAODAN__mac_get_fd(int _fd, mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_get_fd(%i, %p)",_fd,_label);
#endif
	return 0x0;
}

int DAODAN__mac_get_file(const char *_path, mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_get_file(%s, %p)",_path,_label);
#endif
	return 0x0;
}

int DAODAN__mac_get_lcid(pid_t _lcid, mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_get_lcid(%i, %p)",_lcid, _label);
#endif
	return 0x0;
}

int DAODAN__mac_get_lctx(mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_get_lctx(%p)",_label);
#endif
	return 0x0;
}

int DAODAN__mac_get_link(const char *_path, mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_get_link(%s, %p)",_path, _label);
#endif
	return 0x0;
}

int DAODAN__mac_get_pid(pid_t _pid, mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_get_pid(%i, %p)",_pid, _label);
#endif
	return 0x0;
}

int DAODAN__mac_get_proc(mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_get_proc(%p)",_label);
#endif
	return 0x0;
}

int DAODAN__mac_set_fd(int _fildes, const mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_set_fd(%i, %p)",_fildes, _label);
#endif
	return 0x0;
}

int DAODAN__mac_set_file(const char *_path, mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_set_file(%s, %p)",_path, _label);
#endif
	return 0x0;
}

int DAODAN__mac_set_lctx(mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_set_lctx(%p)",_label);
#endif
	return 0x0;
}

int DAODAN__mac_set_link(const char *_path, mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_set_link(%s, %p)",_path,_label);
#endif
	return 0x0;
}

int DAODAN__mac_mount(const char *type, const char *path, int flags, void *data, struct mac *label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_mount(%s, %s, %i, %p, %p)",type,path,flags,data,label);
#endif
	return 0x0;
}

int DAODAN__mac_get_mount(const char *path, struct mac *label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_get_mount(%s, %p)",path,label);
#endif
	return 0x0;
}

int DAODAN__mac_set_proc(const mac_t _label) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_set_proc(%p)",_label);
#endif
	return 0x0;
}

int DAODAN__mac_syscall(const char *_policyname, int _call, void *_arg) {
#if DEBUG
	SDMPrint(PrintCode_NTR,"__mac_syscall(%s, %i, %p)",_policyname,_call,_arg);
#endif
	return 0x0;
}

DYLD_INTERPOSE(DAODAN__mac_execve, __mac_execve);
DYLD_INTERPOSE(DAODAN__mac_get_fd, __mac_get_fd);
DYLD_INTERPOSE(DAODAN__mac_get_file, __mac_get_file);
DYLD_INTERPOSE(DAODAN__mac_get_lcid, __mac_get_lcid);
DYLD_INTERPOSE(DAODAN__mac_get_lctx, __mac_get_lctx);
DYLD_INTERPOSE(DAODAN__mac_get_link, __mac_get_link);
DYLD_INTERPOSE(DAODAN__mac_get_pid, __mac_get_pid);
DYLD_INTERPOSE(DAODAN__mac_get_proc, __mac_get_proc);
DYLD_INTERPOSE(DAODAN__mac_set_fd, __mac_set_fd);
DYLD_INTERPOSE(DAODAN__mac_set_file, __mac_set_file);
DYLD_INTERPOSE(DAODAN__mac_set_lctx, __mac_set_lctx);
DYLD_INTERPOSE(DAODAN__mac_set_link, __mac_set_link);
DYLD_INTERPOSE(DAODAN__mac_mount, __mac_mount);
DYLD_INTERPOSE(DAODAN__mac_get_mount, __mac_get_mount);
DYLD_INTERPOSE(DAODAN__mac_set_proc, __mac_set_proc);
DYLD_INTERPOSE(DAODAN__mac_syscall, __mac_syscall);

/***************************************************/

static struct SDMMOLibrarySymbolTable *binaryTable;

void initDaodan() {
	_dyld_register_func_for_add_image(SDMAddImageHook);
	_dyld_register_func_for_remove_image(SDMRemoveImageHook);
	uint32_t result = SDMGetExecuteImage();
	if (result != 0xffffffff) {
		binaryTable = SDMSTLoadLibrary((char*)_dyld_get_image_name(result), result);
	} else {
		SDMPrint(PrintCode_ERR,"Could not find an executable binary image.");
	}
	if (!binaryTable->couldLoad) {
		SDMPrint(PrintCode_ERR,"Could not load the MachO file, unloading Daodan now...");
		unloadDaodan();
	}
}

void unloadDaodan() {
	SDMPrint(PrintCode_TRY,"Looking for Daodan.");
	void* symbolAddress = NULL;
	symbolAddress = dlsym(RTLD_SELF, "_initDaodan");
	Dl_info libInfo;
	if (dladdr(symbolAddress, &libInfo) == 0) {
		SDMPrint(PrintCode_OK,"Found Daodan.\n");
		void* daodanHandle = dlopen(libInfo.dli_fbase, RTLD_LAZY);
		if (daodanHandle) {
			SDMPrint(PrintCode_OK,"Unloading Daodan.");
		} else {
			SDMPrint(PrintCode_ERR,"Error creating handle to Daodan.");
		}
		SDMSTLibraryRelease(binaryTable);
		dlclose(daodanHandle);
	} else {
		SDMPrint(PrintCode_ERR,"Could not find Daodan.\n");
	}
}

uintptr_t daodanLookupFunction(char *name) {
	SDMPrint(PrintCode_TRY,"Looking up function with name: %s",name);
	struct SDMSTFunction *symbol = SDMSTCreateFunction(binaryTable, name);
	if (symbol->offset) {
		SDMPrint(PrintCode_OK,"Successfully found symbol!");
	} else {
		SDMPrint(PrintCode_ERR,"Could not find symbol with name \"%s\".",name);
	}
	return (uintptr_t)symbol->offset;
}

void daodanLoadSymbolTableForImage(uint32_t index) {
	if (binaryTable)
		SDMSTLibraryRelease(binaryTable);
	binaryTable = SDMSTLoadLibrary((char*)_dyld_get_image_name(index),index);
}

uint32_t SDMGetExecuteImage() {
	SDMPrint(PrintCode_TRY,"Looking for application binary.");
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
	SDMPrint(PrintCode_OK,"Load: %08lx %s",vmaddr_slide,path);
#else
	// this exists for debugging, do not ship in debug mode.
#endif
	free(path);
}

void SDMRemoveImageHook(const struct mach_header* mh, intptr_t vmaddr_slide) {
	char *path = NULL;
	SDMGetImageLocation(mh, &path);
#if DEBUG
	SDMPrint(PrintCode_OK,"Unloaded: %08lx %s",vmaddr_slide,path);
#else
	// this exists for debugging, do not ship in debug mode.
#endif
	free(path);
}

#endif