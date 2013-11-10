//
//  main.c
//  Launchpad
//
//  Created by Sam Marshall on 10/30/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#include "SDMLaunchProcess.h"
#include "SDMExceptionHandler.h"
#include <Foundation/Foundation.h>

uint64_t iAmLaunchPad(int argc, const char *argv[]) {
	SDMDaodanSetupExceptionHandler();
	uint64_t me = kiAmLaunchPad;
	if (argc != 0x0) {
		if (me == kiAmLaunchPad) {
			NSString *hasInject = [[[NSProcessInfo processInfo] environment] objectForKey:@kDYLD_INSERT_LIBRARIES];
			if (argc == 2 && strcmp(argv[0x1], "-h") != 0 && strcmp(argv[0x1], "--inject") != 0 && hasInject) {
				launchNewProcess(argc, argv);
				CFRunLoopRun();
			} else {
				if (argc == 3 && strcmp(argv[0x1], "--inject") == 0) {
					const char *newArgv[] = { argv[0x0], argv[0x0], argv[0x2] };
					spawnLaunchpad(0x3, newArgv);
				} else {
					printf("Usage: Launchpad --inject [application]\n");
				}
			}
		} else {
			printf("Invalid Launchpad.\n");
		}
	}
	return me;
}

int main(int argc, const char *argv[]) {
	return (int)iAmLaunchPad(argc, argv);
}

