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

static mach_port_t server_port;

static void exception_server(mach_port_t exceptionPort);
extern boolean_t mach_exc_server (mach_msg_header_t *msg, mach_msg_header_t *reply);

void SDMDaodanSetupExceptionHandler() {
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0x0), ^{
		kern_return_t kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &server_port);
		if (kr == KERN_SUCCESS) {
			kr = mach_port_insert_right(mach_task_self(), server_port, server_port, MACH_MSG_TYPE_MAKE_SEND);
			if (kr == KERN_SUCCESS) {
				kr = task_set_exception_ports(mach_task_self(), EXC_MASK_BAD_ACCESS, server_port, EXCEPTION_DEFAULT|MACH_EXCEPTION_CODES, THREAD_STATE_NONE);
				if (kr == KERN_SUCCESS) {
					exception_server(mach_task_self());
				}
			}
		}
	});
}

static void exception_server(mach_port_t exceptionPort) {
	mach_msg_return_t rt;
	void* msg;
	void* reply;
	
	msg = calloc(0x1, sizeof(union __RequestUnion__mach_exc_subsystem));
	reply = calloc(0x1, sizeof(union __ReplyUnion__mach_exc_subsystem));
	bool catchMsg = true;
	while (catchMsg) {
		rt = mach_msg((mach_msg_header_t*)msg, MACH_RCV_MSG, 0, sizeof(union __RequestUnion__mach_exc_subsystem), exceptionPort, 0, MACH_PORT_NULL);
		if (rt == MACH_MSG_SUCCESS) {

			mach_exc_server((mach_msg_header_t*)msg, (mach_msg_header_t*)reply);
			
			rt = mach_msg((mach_msg_header_t*)reply, MACH_SEND_MSG, ((mach_msg_header_t*)reply)->msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
			catchMsg = ((rt == KERN_SUCCESS) ? true : false);
		} else {
			catchMsg = false;
		}
		if (!catchMsg) {
			break;
		}
	}
	free(msg);
	free(reply);
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