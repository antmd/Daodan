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

void* exception_server(void *exceptionPort) {
	mach_msg_return_t rt;
	size_t bodySize = sizeof(union __RequestUnion__mach_exc_subsystem);
	mach_msg_header_t* msg = (mach_msg_header_t*)calloc(0x1, bodySize);
	mach_msg_header_t* reply = (mach_msg_header_t*)calloc(0x1, bodySize);
	while (true) {
		((mach_msg_header_t*)msg)->msgh_local_port = (mach_port_t)*(mach_port_t *)exceptionPort;
		((mach_msg_header_t*)msg)->msgh_size = (mach_msg_size_t)bodySize;
		rt = mach_msg((mach_msg_header_t*)msg, MACH_RCV_MSG, 0x0, (mach_msg_size_t)bodySize, (mach_port_t)*(mach_port_t *)exceptionPort, 0x0, MACH_PORT_NULL);
		printf("recv: %08x %s\n",rt,mach_error_string(rt));
			
		mach_exc_server((mach_msg_header_t*)msg, (mach_msg_header_t*)reply);
		
		rt = mach_msg((mach_msg_header_t*)reply, MACH_SEND_MSG, ((mach_msg_header_t*)reply)->msgh_size, 0x0, ((mach_msg_header_t*)msg)->msgh_local_port, 0x0, MACH_PORT_NULL);
		printf("send: %08x %s\n",rt,mach_error_string(rt));
	}
	printf("leaving handler");
	free(msg);
	free(reply);
	pthread_exit(0x0);
}

#pragma mark -
#pragma mark Exception Handlers

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