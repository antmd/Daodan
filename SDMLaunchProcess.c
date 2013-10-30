//
//  SDMLaunchProcess.c
//  libDaodan
//
//  Created by Sam Marshall on 10/30/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMLaunchProcess_c
#define Daodan_SDMLaunchProcess_c

#include "SDMLaunchProcess.h"
#include <sys/types.h>
#include <sys/ptrace.h>
#include <mach/mach.h>
#include <Security/Security.h>

int acquireTaskForPortRight() {
	AuthorizationRef authorization;
	OSStatus status = AuthorizationCreate (NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &authorization);
	if (status != 0) {
		fprintf(stderr, "Error creating authorization reference\n");
		return -1;
	}
	AuthorizationItem right = { "system.privilege.taskport", 0, 0, 0 };
	AuthorizationItem items[] = { right };
	AuthorizationRights rights = { sizeof(items) / sizeof(items[0]), items };
	AuthorizationFlags flags = kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize;
	
	status = AuthorizationCopyRights (authorization, &rights, kAuthorizationEmptyEnvironment, flags, NULL);
	if (status != 0) {
		fprintf(stderr, "Error authorizing current process with right to call task_for_pid\n");
		return -1;
	}
	return 0;
}

void launchNewProcess(int argc, const char *argv[]) {
	for (uint32_t i = 0x0; i < argc; i++) {
		printf("%i %s\n",i,argv[i]);
	}
	pid_t child = fork();
	if (child < 0x0) {
		printf("fork() failed: %s\n",strerror(errno));
	} else if (child == 0x0) {
		long ptraceResult = ptrace(PT_TRACE_ME, 0, 0, 0);
		if (ptraceResult != 0x0) {
			printf("PT_TRACE_ME failed.\n");
		} else {
			if (argc == 0x2) {
				execl(argv[0x1], argv[0x1], NULL);
			} else {
				execl(argv[0x1], argv[0x1], argv[0x2], NULL);
			}
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
					x86_thread_state64_t state;
					mach_msg_type_number_t stateCount = x86_THREAD_STATE64_COUNT;
					result = thread_get_state(threadList[0], x86_THREAD_STATE64, (thread_state_t)&state,&stateCount);
					if (result == KERN_SUCCESS) {
						long ptraceResult = ptrace(PT_CONTINUE, child, (char*)1, 0);
						if (ptraceResult != 0x0) {
							printf("PT_CONTINUE failed.\n");
							exit(-1);
						}
					} else {
						printf("thread_get_state failed. %s\n",mach_error_string(result));
						exit(-1);
					}
				} else {
					printf("task_threads failed. %s\n",mach_error_string(result));
					exit(-1);
				}
			} else {
				printf("task_for_pid failed. %s\n",mach_error_string(result));
				exit(-1);
			}
		} else {
			printf("acquireTaskportRight failed.\n");
			exit(-1);
		}
    }
}

#endif