//
//  main.c
//  Launchpad
//
//  Created by Sam Marshall on 10/30/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#include "SDMLaunchProcess.h"
#include "SDMExceptionHandler.h"
#include "SDMSymbolTable.h"
#include "DaodanStore.h"
#include <Foundation/Foundation.h>

static int launchpad_notify_token;

uint64_t SDMLaunchpadDisplayHelp() {
    printf("Usage: Launchpad\n");
    printf("\t--inject [application]\n");
    printf("\t\tInject libDaodan into the target application.\n\n");
    printf("\t--dump [application]\n");
    printf("\t\tDump the target application using libDaodan.\n\n");
    return 0x0;
}

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
		if (me == kiAmLaunchPad) {
			NSString *hasInject = [[[NSProcessInfo processInfo] environment] objectForKey:@kDYLD_INSERT_LIBRARIES];
			if ((strcmp(argv[0x1], "-h") == 0) || (strcmp(argv[0x1], "--help") == 0)) {
				me = SDMLaunchpadDisplayHelp();
			}
			if (argc == 0x3) {
				if (strcmp(argv[0x1], "--inject") == 0x0) {
                    SDMDaodanSetupExceptionHandler();
					if (hasInject) {
						launchNewProcess(argc, argv);
					} else {
						spawnLaunchpad(argc, argv);
						notify_post(GenerateLaunchpadKillswitchName());
					}
				} else if (strcmp(argv[0x1], "--dump") == 0x0) {
					char *path = Ptr(argv[0x2]);
                    struct SDMSTBinary *binaryTable = SDMSTLoadBinaryFromFilePath(path);
					uint64_t binaryOffset = 0x0;
					for (uint32_t i = 0x0; i < binaryTable->archCount; i++) {
						Pointer archOffset = (Pointer)PtrAdd(binaryTable->handle, binaryTable->arch[i]);
						struct SDMSTLibrary *libTable = SDMSTDumpBinaryArch(path, archOffset, binaryOffset, false);
						if (libTable) {
							SDMDaodanWriteDump(libTable);
							SDMSTLibraryRelease(libTable);
						}
						binaryOffset = (uint64_t)(binaryTable->arch[i]);
					}
					SDMSTBinaryRelease(binaryTable);
					me = 0x0;
                }
			}
		} else {
			printf("Invalid Launchpad.\n");
		}
	} else {
        me = SDMLaunchpadDisplayHelp();
    }
	return me;
}

int main(int argc, const char *argv[]) {
	int returnValue = (int)iAmLaunchPad(argc, argv);
	return returnValue;
}

