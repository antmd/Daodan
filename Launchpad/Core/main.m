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

static int launchpad_notify_token;

void SDMLaunchpadSetupKillswitch() {
	char *killswitch = GenerateLaunchpadKillswitchName();
	notify_register_dispatch(killswitch, &launchpad_notify_token, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0x0), ^(int token){
		kill(getpid(), 0x9);
	});
}

uint64_t iAmLaunchPad(int argc, const char *argv[]) {
	uint64_t me = kiAmLaunchPad;
	if ((argc != 0x0) && (argc > 0x1)) {
		SDMLaunchpadSetupKillswitch();
		SDMDaodanSetupExceptionHandler();
		if (me == kiAmLaunchPad) {
			NSString *hasInject = [[[NSProcessInfo processInfo] environment] objectForKey:@kDYLD_INSERT_LIBRARIES];
			if ((strcmp(argv[0x1], "-h") == 0) || (strcmp(argv[0x1], "--help") == 0)) {
				printf("Usage: Launchpad --inject [application]\n");
			}
			if (argc == 0x3) {
				if (strcmp(argv[0x1], "--inject") == 0) {
					if (hasInject) {
						launchNewProcess(argc, argv);
					} else {
						spawnLaunchpad(argc, argv);
						notify_post(GenerateLaunchpadKillswitchName());
					}
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

