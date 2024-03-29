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
#include <security/mac.h>
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <mach/mach_vm.h>
#include <mach/mach.h>

#include "SDMLaunchProcess.h"
#include "DaodanStore.h"

#include "SDMDaodanPayload.h"
#include "SDMExceptionHandler.h"

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
	LogPrint(PrintCode_NTR,"__mac_execve(%s, %p, %p, %p)",fname,argv,envv,_label);
	return 0x0;
}

int DAODAN__mac_get_fd(int _fd, mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_get_fd(%i, %p)",_fd,_label);
	return 0x0;
}

int DAODAN__mac_get_file(const char *_path, mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_get_file(%s, %p)",_path,_label);
	return 0x0;
}

int DAODAN__mac_get_lcid(pid_t _lcid, mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_get_lcid(%i, %p)",_lcid, _label);
	return 0x0;
}

int DAODAN__mac_get_lctx(mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_get_lctx(%p)",_label);
	return 0x0;
}

int DAODAN__mac_get_link(const char *_path, mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_get_link(%s, %p)",_path, _label);
	return 0x0;
}

int DAODAN__mac_get_pid(pid_t _pid, mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_get_pid(%i, %p)",_pid, _label);
	return 0x0;
}

int DAODAN__mac_get_proc(mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_get_proc(%p)",_label);
	return 0x0;
}

int DAODAN__mac_set_fd(int _fildes, const mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_set_fd(%i, %p)",_fildes, _label);
	return 0x0;
}

int DAODAN__mac_set_file(const char *_path, mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_set_file(%s, %p)",_path, _label);
	return 0x0;
}

int DAODAN__mac_set_lctx(mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_set_lctx(%p)",_label);
	return 0x0;
}

int DAODAN__mac_set_link(const char *_path, mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_set_link(%s, %p)",_path,_label);
	return 0x0;
}

int DAODAN__mac_mount(const char *type, const char *path, int flags, void *data, struct mac *label) {
	LogPrint(PrintCode_NTR,"__mac_mount(%s, %s, %i, %p, %p)",type,path,flags,data,label);
	return 0x0;
}

int DAODAN__mac_get_mount(const char *path, struct mac *label) {
	LogPrint(PrintCode_NTR,"__mac_get_mount(%s, %p)",path,label);
	return 0x0;
}

int DAODAN__mac_set_proc(const mac_t _label) {
	LogPrint(PrintCode_NTR,"__mac_set_proc(%p)",_label);
	return 0x0;
}

int DAODAN__mac_syscall(const char *_policyname, int _call, void *_arg) {
	LogPrint(PrintCode_NTR,"__mac_syscall(%s, %i, %p)",_policyname,_call,_arg);
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

static struct SDMSTLibrary *binaryTable;

#define DC_LAUNCH "com.samdmarshall.DaodanController.Launch"
#define DC_RELOAD "com.samdmarshall.DaodanController.ReloadPlugins"
#define DC_QUIT "com.samdmarshall.DaodanController.Quit"

typedef enum DaodanNotificationListenerType {
	DaodanNotificationListenerTypeLaunch = 0x0,
	DaodanNotificationListenerTypeReload = 0x1,
	DaodanNotificationListenerTypeQuit = 0x2,
	DaodanNotificationListenerTypeTotal
} DaodanNotificationListenerType;

static int daodan_notify_token[DaodanNotificationListenerTypeTotal];

#define DAODAN_RECV_LENGTH (sizeof(struct DaodanMachMessage)+sizeof(mach_msg_header_t))

#define MACH_SEND_ENABLED false

static mach_port_name_t portSend = MACH_PORT_NULL;
static dispatch_source_t dispatchSend = NULL;
static dispatch_queue_t dispatchSendQueue = NULL;

static mach_port_name_t portReceive = MACH_PORT_NULL;
static dispatch_source_t dispatchReceive = NULL;
static dispatch_queue_t dispatchReceiveQueue = NULL;

static mach_msg_id_t messageCounter = 0x0;

static uint32_t daodanExecutableImageIndex = 0x0;

// SDM: TODO: create encode and decode functions for processing data in MachMessage

uint32_t sendDaodanMachMessage(char data[0x400]) {
	struct DaodanMachMessage message;
	memcpy(&(message.data[0x0]), &(data[0x0]), 0x400);
	message.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MAKE_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	message.header.msgh_remote_port = portReceive;
	message.header.msgh_local_port = portSend;
	message.header.msgh_id = messageCounter;
	message.header.msgh_size = sizeof(struct DaodanMachMessage);
	uint32_t result = mach_msg(&message.header, MACH_SEND_MSG, sizeof(struct DaodanMachMessage), 0x0, portReceive, MACH_SEND_TIMEOUT, MACH_PORT_NULL);
	messageCounter++;
	if (result == KERN_SUCCESS) {
		return 0x0;
	} else {
		return result;
	}
}

void setupDaodanControllerNotificationListeners() {
	uint32_t result[DaodanNotificationListenerTypeTotal];
	result[DaodanNotificationListenerTypeLaunch] = notify_register_dispatch(DC_LAUNCH, &daodan_notify_token[DaodanNotificationListenerTypeLaunch], dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0x0), ^(int token){
		char data[0x400];
		sprintf(&(data[0x0]),"DaodanController is launching!");
		sendDaodanMachMessage(data);
	});
	result[DaodanNotificationListenerTypeReload] = notify_register_dispatch(DC_RELOAD, &daodan_notify_token[DaodanNotificationListenerTypeReload], dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0x0), ^(int token){
		char data[0x400];
		sprintf(&(data[0x0]),"Reload Plugins!");
		sendDaodanMachMessage(data);
	});
	result[DaodanNotificationListenerTypeQuit] = notify_register_dispatch(DC_QUIT, &daodan_notify_token[DaodanNotificationListenerTypeQuit], dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0x0), ^(int token){
		char data[0x400];
		sprintf(&(data[0x0]),"DaodanController is quitting!");
		sendDaodanMachMessage(data);
	});
	bool statusOK = true;
	for (uint32_t i = 0x0; i < DaodanNotificationListenerTypeTotal; i++) {
		if (result[i] != NOTIFY_STATUS_OK) {
			statusOK = false;
			break;
		}
	}
	if (statusOK) {
		SDMFormatPrint(false,PrintCode_OK,"Successfully registered notify listeners");
	} else {
		SDMFormatPrint(false,PrintCode_ERR,"Unable to registery notify listeners, Daodan will be unable to communicate with DaodanController");
	}
}

void cancelDaodanControllerNotificationListeners() {
	for (uint32_t i = 0x0; i < DaodanNotificationListenerTypeTotal; i++) {
		notify_cancel(daodan_notify_token[i]);
	}
}

static dispatch_block_t portSendHandler = ^{
	struct DaodanMachMessage *message = calloc(0x1, sizeof(struct DaodanMachMessage));
	message->header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MAKE_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	message->header.msgh_remote_port = portReceive;
	message->header.msgh_local_port = portSend;
	message->header.msgh_id = 0x0;
	message->header.msgh_size = sizeof(struct DaodanMachMessage);
	uint32_t result = mach_msg(&(message->header), MACH_SEND_MSG, sizeof(struct DaodanMachMessage), 0x0, portReceive, MACH_SEND_TIMEOUT, MACH_PORT_NULL);
	if (result != KERN_SUCCESS) {
		SDMFormatPrint(false,PrintCode_ERR,"error %08x %s",result,mach_error_string(result));
	}
	printf("+++%s\n",&(message->data[0x0]));
	free(message);
};

static dispatch_block_t portReceiveHandler = ^{
	struct DaodanMachMessage *message = calloc(0x1, DAODAN_RECV_LENGTH);
	message->header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MOVE_RECEIVE, MACH_MSG_TYPE_COPY_RECEIVE);
	message->header.msgh_remote_port = portSend;
	message->header.msgh_local_port = portReceive;
	message->header.msgh_id = 0x0;
	message->header.msgh_size = DAODAN_RECV_LENGTH;
	uint32_t result = mach_msg(&(message->header), MACH_RCV_MSG, 0x0, DAODAN_RECV_LENGTH, portReceive, MACH_RCV_TIMEOUT, MACH_PORT_NULL);
	if (result != KERN_SUCCESS) {
		SDMFormatPrint(false,PrintCode_ERR,"error %08x %s",result,mach_error_string(result));
	}
	printf("---%s\n",&(message->data[0x0]));
	free(message);
};

char* GenerateUniqueQueueName(enum DAODAN_QUEUE type) {
	char *name = calloc(0x400, sizeof(char));
	char *typeDesc = "UNKNOWN";
	switch (type) {
		case DAODAN_MACH_SEND: {
			typeDesc = "MACH_SEND";
			break;
		};
		case DAODAN_MACH_RECV: {
			typeDesc = "MACH_RECV";
			break;
		};
		default: {
			break;
		};
	}
	sprintf(name, "com.samdmarshall.libDaodan.%i.%s",getpid(),typeDesc);
	return name;
}

void setupDaodanMachPort() {
	vm_map_t task = mach_task_self();
	
	kern_return_t resultPortSend = mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &portSend);
	if (resultPortSend == KERN_SUCCESS) {
		char *name = GenerateUniqueQueueName(DAODAN_MACH_SEND);
		dispatchSendQueue = dispatch_queue_create(name, DISPATCH_QUEUE_SERIAL);
		free(name);
		dispatchSend = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_SEND, portSend, 0x0, dispatchSendQueue);
		dispatch_source_set_event_handler(dispatchSend, portSendHandler);
		dispatch_source_set_cancel_handler(dispatchSend, ^{
			mach_port_mod_refs(mach_task_self(), portSend, MACH_PORT_RIGHT_RECEIVE, k32BitMask);
			dispatch_release(dispatchSendQueue);
			dispatch_release(dispatchSend);
			mach_port_deallocate(mach_task_self(), portSend);
		});
		dispatch_resume(dispatchSend);
		SDMFormatPrint(false,PrintCode_OK,"Successfully acquired local mach port for sending");
	} else {
		SDMFormatPrint(false,PrintCode_ERR,"Send mach port setup failed");
	}
	
	kern_return_t resultPortRecieve = mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &portReceive);
	if (resultPortRecieve == KERN_SUCCESS) {
		char *name = GenerateUniqueQueueName(DAODAN_MACH_RECV);
		dispatchReceiveQueue = dispatch_queue_create(name, DISPATCH_QUEUE_SERIAL);
		free(name);
		dispatchReceive = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, portReceive, 0x0, dispatchReceiveQueue);
		dispatch_source_set_event_handler(dispatchReceive, portReceiveHandler);
		dispatch_source_set_cancel_handler(dispatchReceive, ^{
			mach_port_mod_refs(mach_task_self(), portReceive, MACH_PORT_RIGHT_RECEIVE, k32BitMask);
			dispatch_release(dispatchReceiveQueue);
			dispatch_release(dispatchReceive);
			mach_port_deallocate(mach_task_self(), portReceive);
			SDMFormatPrint(false,PrintCode_OK,"Releasing receive port handler");
		});
		dispatch_resume(dispatchReceive);
		SDMFormatPrint(false,PrintCode_OK,"Successfully acquired local mach port for receiving");
	} else {
		SDMFormatPrint(false,PrintCode_ERR,"Receive mach port setup failed");
	}
}

void setupDaodanControllerMachPort(pid_t controllerPID) {
	vm_map_t task = mach_task_self();
	mach_port_t target;
	// get pid and store here
	kern_return_t taskResult = task_for_pid(task, controllerPID, &target);
	if (taskResult == KERN_SUCCESS) {
		// do stuff
		SDMFormatPrint(false,PrintCode_OK,"Successfully called task_for_pid(%i) back to DaodanController",controllerPID);
	} else {
		SDMFormatPrint(false,PrintCode_ERR,"Unable to attach task_for_pid(%i), error: %s.",controllerPID,mach_error_string(taskResult));
		SDMFormatPrint(false,PrintCode_ERR,"Please relaunch using Launchpad");
	}
}

void closeDaodanMachPorts() {
	if (dispatchSend)
		dispatch_source_cancel(dispatchSend);
	if (dispatchReceive)
		dispatch_source_cancel(dispatchReceive);
}

bool locateLaunchpad() {
	bool found = false;
	SDMSTFunctionCall symbolAddress = SDMSTSymbolLookup(binaryTable, "_iAmLaunchPad");
	if (symbolAddress) {
		uint64_t result = (uint64_t)symbolAddress(NULL, NULL);
		if (result == kiAmLaunchPad) {
			LogPrint(PrintCode_OK,"Found Launchpad");
			found = true;
		}
	}
	return found;
}

void dumpDaodan() {
	SDMDaodanWriteDump(binaryTable);
}

void initDaodan() {
	_dyld_register_func_for_add_image(SDMAddImageHook);
	_dyld_register_func_for_remove_image(SDMRemoveImageHook);
	uint32_t result = daodanExecutableImageIndex = SDMGetExecuteImage();
	if (result != k32BitMask) {
		binaryTable = SDMSTLoadLibrary(Ptr(_dyld_get_image_name(result)), result, false);
	} else {
		SDMFormatPrint(false,PrintCode_ERR,"Could not find an executable binary image.");
	}
	if (!(binaryTable->couldLoad)) {
		SDMFormatPrint(false,PrintCode_ERR,"Could not load the MachO file, unloading Daodan now...");
		unloadDaodan();
	} else {
		SDMDaodanSetupExceptionHandler();
		bool foundLaunchpad = locateLaunchpad();
		if (foundLaunchpad) {
			SDMFormatPrint(false,PrintCode_NTR,"Unloading");
			unloadDaodan();
		} else {
			SDMFormatPrint(false,PrintCode_TRY,"Registering notify listeners for DaodanController...");
			setupDaodanControllerNotificationListeners();
			setupDaodanMachPort();
			setupDaodanControllerMachPort(181);
		}
	}
}

void unloadDaodan() {
	SDMSTLibraryRelease(binaryTable);
	cancelDaodanControllerNotificationListeners();
	closeDaodanMachPorts();
}

uintptr_t daodanLookupFunction(char *name) {
	LogPrint(PrintCode_TRY,"Looking up function with name: %s",name);
	struct SDMSTFunction *symbol = SDMSTCreateFunction(binaryTable, name);
	if (symbol->offset) {
		SDMFormatPrint(false,PrintCode_OK,"Successfully found symbol!");
	} else {
		SDMFormatPrint(false,PrintCode_ERR,"Could not find symbol with name \"%s\".",name);
	}
	return (uintptr_t)(symbol->offset);
}

void daodanLoadSymbolTableForImage(uint32_t index) {
	if (binaryTable)
		SDMSTLibraryRelease(binaryTable);
	binaryTable = SDMSTLoadLibrary(Ptr(_dyld_get_image_name(index)),index, false);
}

uint32_t SDMGetIndexForLibraryPath(char *path) {
	uint32_t index;
	for (index = 0x0; index < _dyld_image_count(); index++) {
		if (strcmp(path, _dyld_get_image_name(index)))
			break;
	}
	return index;
}

uint32_t SDMGetExecuteImage() {
	LogPrint(PrintCode_TRY,"Looking for application binary...");
	struct mach_header *imageHeader;
	uint32_t count = _dyld_image_count();
	bool foundBinary = false;
	uint32_t index;
	for (index = 0x0; index < count; index++) {
		imageHeader = (struct mach_header *)_dyld_get_image_header(index);
		if (imageHeader->filetype == MH_EXECUTE) {
			foundBinary = true;
			break;
		}
	}
	return (foundBinary ? index : k32BitMask);
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
	LogPrint(PrintCode_OK,"Load: %08lx %s",vmaddr_slide,path);
	free(path);
}

void SDMRemoveImageHook(const struct mach_header* mh, intptr_t vmaddr_slide) {
	char *path = NULL;
	SDMGetImageLocation(mh, &path);
	LogPrint(PrintCode_OK,"Unloaded: %08lx %s",vmaddr_slide,path);
	free(path);
}

#endif