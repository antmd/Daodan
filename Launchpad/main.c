//
//  main.c
//  Launchpad
//
//  Created by Sam Marshall on 10/30/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#include "SDMLaunchProcess.h"
#include <CoreFoundation/CoreFoundation.h>

uint64_t iAmLaunchPad(int argc, const char *argv[]) {
	uint64_t me = kiAmLaunchPad;
	if (argc != 0x0) {
		if (me == kiAmLaunchPad) {
			if (argc == 2 && strcmp(argv[0x1], "-h") != 0) {
				launchNewProcess(argc, argv);
				CFRunLoopRun();
			} else {
				printf("Usage: Launchpad [application]\n");
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

