//
//  SDMExceptionHandler.c
//  Daodan
//
//  Created by Sam Marshall on 11/10/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMExceptionHandler_c
#define Daodan_SDMExceptionHandler_c

#include "SDMExceptionHandler.h"
#include "SDMLaunchProcess.h"
#include "SDMSymbolTable.h"
#include "mach_exc.h"
#include "mach_excServer.h"
#include "exc.h"
#include "excServer.h"
#include <mach/mach_error.h>
#include <dispatch/dispatch.h>
#include <pthread.h>

void* exception_server(void *exceptionPort);
extern boolean_t mach_exc_server(mach_msg_header_t *msg, mach_msg_header_t *reply);

void SDMDaodanSetupExceptionHandler() {
	mach_port_t server_port = MACH_PORT_NULL;
	pthread_t exceptionThread;
	kern_return_t kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &server_port);
	if (kr == KERN_SUCCESS) {
		kr = mach_port_insert_right(mach_task_self(), server_port, server_port, MACH_MSG_TYPE_MAKE_SEND);
		if (kr == KERN_SUCCESS) {
			kr = task_set_exception_ports(mach_task_self(), EXC_MASK_ALL, server_port, EXCEPTION_DEFAULT, THREAD_STATE_NONE);
			if (kr == KERN_SUCCESS) {
				pthread_create(&exceptionThread, NULL, exception_server, (void*)&server_port);
				pthread_detach(exceptionThread);
			}
		}
	}
}

void SignalHandler(int signal) {
	printf("signal %i was raised!\n",signal);
	switch (signal) {
		case SIGHUP: {
			break;
		};
		case SIGINT: {
			break;
		};
		case SIGQUIT: {
			break;
		};
		case SIGILL: {
			break;
		};
		case SIGTRAP: {
			break;
		};
		case SIGABRT: {
			break;
		};
		case SIGEMT: {
			break;
		};
		case SIGFPE: {
			break;
		};
		case SIGKILL: {
			break;
		};
		case SIGBUS: {
			break;
		};
		case SIGSEGV: {
			break;
		};
		case SIGSYS: {
			break;
		};
		case SIGPIPE: {
			break;
		};
		case SIGALRM: {
			break;
		};
		case SIGTERM: {
			break;
		};
		case SIGURG: {
			break;
		};
		case SIGSTOP: {
			break;
		};
		case SIGTSTP: {
			break;
		};
		case SIGCONT: {
			break;
		};
		case SIGCHLD: {
			break;
		};
		case SIGTTIN: {
			break;
		};
		case SIGTTOU: {
			break;
		};
		case SIGIO: {
			break;
		};
		case SIGXCPU: {
			break;
		};
		case SIGXFSZ: {
			break;
		};
		case SIGVTALRM: {
			break;
		};
		case SIGPROF: {
			break;
		};
		case SIGWINCH: {
			break;
		};
		case SIGINFO: {
			break;
		};
		case SIGUSR1: {
			break;
		};
		case SIGUSR2: {
			break;
		};
		default: {
			break;
		};
	}
}

void SignalHandlerSetup() {
	signal(SIGHUP, SignalHandler);
	signal(SIGINT, SignalHandler);
	signal(SIGQUIT, SignalHandler);
	signal(SIGILL, SignalHandler);
	signal(SIGTRAP, SignalHandler);
	signal(SIGABRT, SignalHandler);
	signal(SIGIOT, SignalHandler);
	signal(SIGEMT, SignalHandler);
	signal(SIGFPE, SignalHandler);
	signal(SIGKILL, SignalHandler);
	signal(SIGBUS, SignalHandler);
	signal(SIGSEGV, SignalHandler);
	signal(SIGSYS, SignalHandler);
	signal(SIGPIPE, SignalHandler);
	signal(SIGALRM, SignalHandler);
	signal(SIGTERM, SignalHandler);
	signal(SIGURG, SignalHandler);
	signal(SIGSTOP, SignalHandler);
	signal(SIGTSTP, SignalHandler);
	signal(SIGCONT, SignalHandler);
	signal(SIGCHLD, SignalHandler);
	signal(SIGTTIN, SignalHandler);
	signal(SIGTTOU, SignalHandler);
	signal(SIGIO, SignalHandler);
	signal(SIGXCPU, SignalHandler);
	signal(SIGXFSZ, SignalHandler);
	signal(SIGVTALRM, SignalHandler);
	signal(SIGPROF, SignalHandler);
	signal(SIGWINCH, SignalHandler);
	signal(SIGINFO, SignalHandler);
	signal(SIGUSR1, SignalHandler);
	signal(SIGUSR2, SignalHandler);
}

void* exception_server(void *exceptionPort) {
	SignalHandlerSetup();
	
	mach_port_t taskPort = (mach_port_t)*(mach_port_t *)exceptionPort;
	mach_msg_return_t rt;
	mach_msg_size_t bodySize = sizeof(mach_msg_header_t)+0x100;
	mach_msg_header_t* msg = (mach_msg_header_t*)calloc(0x1, bodySize);
	mach_msg_header_t* reply = (mach_msg_header_t*)calloc(0x1, bodySize);
	while (true) {
		((mach_msg_header_t*)msg)->msgh_local_port = taskPort;
		((mach_msg_header_t*)msg)->msgh_size = bodySize;
		rt = mach_msg((mach_msg_header_t*)msg, MACH_RCV_MSG, 0x0, bodySize, taskPort, 0x0, MACH_PORT_NULL);
		if (rt != KERN_SUCCESS) {
			printf("recv: %08x %s\n",rt,mach_error_string(rt));
		}
		
		struct SDMSTLibraryArchitecture arch = SDMSTGetArchitecture();
		if (arch.type == CPU_TYPE_X86_64) {
			exc_server((mach_msg_header_t*)msg, (mach_msg_header_t*)reply);
		} else if (arch.type == CPU_TYPE_I386) {
			mach_exc_server((mach_msg_header_t*)msg, (mach_msg_header_t*)reply);
		}
		
		rt = mach_msg((mach_msg_header_t*)reply, MACH_SEND_MSG, ((mach_msg_header_t*)reply)->msgh_size, 0x0, ((mach_msg_header_t*)msg)->msgh_local_port, 0x0, MACH_PORT_NULL);
		if (rt != KERN_SUCCESS) {
			printf("send: %08x %s\n",rt,mach_error_string(rt));
		}
	}
	printf("leaving handler");
	free(msg);
	free(reply);
	pthread_exit(0x0);
}

#pragma mark -
#pragma mark Exception Handlers

kern_return_t SDM_Daodan_exception_handler();

#pragma mark 64bit
kern_return_t catch_mach_exception_raise(mach_port_t exception_port, mach_port_t thread, mach_port_t task, exception_type_t exception, mach_exception_data_t code, mach_msg_type_number_t codeCnt) {
	printf("My exception handler was called by exception_raise() %i\n",codeCnt);
	return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state(mach_port_t exception_port, exception_type_t exception, const mach_exception_data_t code, mach_msg_type_number_t codeCnt, int *flavor, const thread_state_t old_state, mach_msg_type_number_t old_stateCnt, thread_state_t new_state, mach_msg_type_number_t *new_stateCnt) {
	printf("My exception handler was called by exception_raise_raise_state() %i\n",codeCnt);
    return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state_identity(mach_port_t exception_port, mach_port_t thread, mach_port_t task, exception_type_t exception, mach_exception_data_t code, mach_msg_type_number_t codeCnt, int *flavor, thread_state_t old_state, mach_msg_type_number_t old_stateCnt, thread_state_t new_state, mach_msg_type_number_t *new_stateCnt) {
	printf("My exception handler was called by exception_raise_state_identity() %i\n",codeCnt);
	return KERN_FAILURE;
}

#pragma mark 32bit
kern_return_t catch_exception_raise(mach_port_t exception_port, mach_port_t thread, mach_port_t task, exception_type_t exception, exception_data_t code, mach_msg_type_number_t codeCnt) {
	printf("My exception handler was called by exception_raise() %i\n",codeCnt);
    return KERN_FAILURE;
}

kern_return_t catch_exception_raise_state(mach_port_t exception_port, exception_type_t exception, const exception_data_t code, mach_msg_type_number_t codeCnt, int *flavor, const thread_state_t old_state, mach_msg_type_number_t old_stateCnt, thread_state_t new_state, mach_msg_type_number_t *new_stateCnt) {
	printf("My exception handler was called by exception_raise_state() %i\n",codeCnt);
    return KERN_FAILURE;
}

kern_return_t catch_exception_raise_state_identity(mach_port_t exception_port, mach_port_t thread, mach_port_t task, exception_type_t exception, exception_data_t code, mach_msg_type_number_t codeCnt, int *flavor, thread_state_t old_state, mach_msg_type_number_t old_stateCnt, thread_state_t new_state, mach_msg_type_number_t *new_stateCnt) {
	printf("My exception handler was called by exception_raise_state_identity() %i\n",codeCnt);
	return KERN_FAILURE;
}

#endif