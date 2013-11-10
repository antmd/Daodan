//
//  SDMLaunchProcess.m
//  libDaodan
//
//  Created by Sam Marshall on 10/30/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMLaunchProcess_m
#define Daodan_SDMLaunchProcess_m

#include "SDMLaunchProcess.h"
#include "SDMSymbolTable.h"
#include <sys/types.h>
#include <sys/ptrace.h>
#include <mach/mach.h>
#include <dlfcn.h>
#import <Security/Security.h>
#import <Foundation/Foundation.h>

char* GenerateLaunchpadKillswitchName() {
	pid_t currentPid = getpid();
	char *buffer = calloc(0x1, sizeof(char)*(strlen(kLaunchPadKill)+0x2+SDMGetNumberOfDigits(currentPid)));
	sprintf(buffer,"%s.%i",kLaunchPadKill,currentPid);
	return buffer;
}

int acquireTaskForPortRight() {
	AuthorizationRef authorization;
	OSStatus status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &authorization);
	if (status != 0x0) {
		printf("Error creating authorization reference\n");
		return 0xffffffff;
	}
	AuthorizationItem systemRight = { kAuthorizationRightExecute, 0x0, 0x0, 0x0 };
	AuthorizationItem taskRight = { "system.privilege.taskport", 0x0, 0x0, 0x0 };
	AuthorizationItem items[] = { systemRight, taskRight };
	AuthorizationRights rights = { 0x2, items };
	AuthorizationFlags flags = kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize;
	
	status = AuthorizationCopyRights(authorization, &rights, NULL, flags, NULL);
	if (status != 0x0) {
		printf("Error authorizing current process with right to call task_for_pid()\n");
		notify_post(GenerateLaunchpadKillswitchName());
		return 0xffffffff;
	}
	return 0x0;
}

void spawnLaunchpad(int argc, const char *argv[]) {
	if (argc == 0x3) {
		struct SDMSTLibraryArchitecture arch = SDMSTGetArchitecture();
		NSString *architecture = @"invalid";
		if (arch.type == CPU_TYPE_X86_64) {
			architecture = @"x86_64";
		} else if (arch.type == CPU_TYPE_I386) {
			architecture = @"i386";
		}
		NSTask *task = [[NSTask alloc] init];
		NSMutableDictionary *enviDict = [[NSMutableDictionary alloc] init];
		[enviDict addEntriesFromDictionary:[[NSProcessInfo processInfo] environment]];
		[enviDict setObject:@kDaodanPath forKey:@kDYLD_INSERT_LIBRARIES];
		[enviDict setObject:architecture forKey:@kDAODAN_RUN_ARCH];
		[task setEnvironment:enviDict];
		[task setLaunchPath:@kLaunchPadPath];
		[task setArguments:[NSArray arrayWithObjects:@"--inject",[NSString stringWithFormat:@"%s",argv[0x2]],nil]];
		[task launch];
		[task release];
		[enviDict release];
	}
}

void launchNewProcess(int argc, const char *argv[]) {
	pid_t child = fork();
	if (child < 0x0) {
		printf("fork() failed: %s\n",strerror(errno));
	} else if (child == 0x0) {
		long ptraceResult = ptrace(PT_TRACE_ME, 0x0, 0x0, 0x0);
		if (ptraceResult != 0x0) {
			printf("PT_TRACE_ME failed.\n");
		} else {
			execl(argv[0x2], argv[0x2], NULL);
		}
	} else {
		
		mach_port_t task;
		int status = acquireTaskForPortRight();
		if (status == 0x0) {
			kern_return_t result = task_for_pid(mach_task_self(), child, &task);
			if (result == KERN_SUCCESS) {
				thread_act_port_array_t threadList;
				mach_msg_type_number_t threadCount;
				result = task_threads(task, &threadList, &threadCount);
				if (result == KERN_SUCCESS) {
					int waitStatus;
					wait(&waitStatus);
					mach_msg_type_number_t stateCount;
					NSString *runArch = [[[NSProcessInfo processInfo] environment] objectForKey:@kDAODAN_RUN_ARCH];
					struct SDMSTLibraryArchitecture arch = {0x0,0x0};
					if (runArch) {
						if ([runArch isEqualToString:@"x86_64"]) {
							arch.type = CPU_TYPE_X86_64;
						} else if ([runArch isEqualToString:@"i386"]) {
							arch.type = CPU_TYPE_I386;
						} else {
							arch = SDMSTGetArchitecture();
						}
					} else {
						arch = SDMSTGetArchitecture();
					}
					if (arch.type == CPU_TYPE_X86_64) {
						stateCount = x86_THREAD_STATE64_COUNT;
						x86_thread_state64_t state;
						result = thread_get_state(threadList[0x0], x86_THREAD_STATE64, (thread_state_t)&state, &stateCount);
					} else if (arch.type == CPU_TYPE_I386) {
						stateCount = x86_THREAD_STATE32_COUNT;
						x86_thread_state32_t state;
						result = thread_get_state(threadList[0x0], x86_THREAD_STATE32, (thread_state_t)&state, &stateCount);
					}
					if (result == KERN_SUCCESS) {
						long ptraceResult = ptrace(PT_CONTINUE, child, (char*)0x1, 0x0);
						if (ptraceResult != 0x0) {
							printf("PT_CONTINUE failed.\n");
							exit(0xffffffff);
						} else {
							printf("Successfully launched new process! %s\n",argv[0x2]);
						}
					} else {
						printf("thread_get_state failed. %s\n",mach_error_string(result));
						exit(0xffffffff);
					}
				} else {
					printf("task_threads failed. %s\n",mach_error_string(result));
					exit(0xffffffff);
				}
			} else {
				printf("task_for_pid failed. %s\n",mach_error_string(result));
				exit(0xffffffff);
			}
		} else {
			printf("acquireTaskportRight failed.\n");
			exit(0xffffffff);
		}
    }
}

#endif