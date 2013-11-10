//
//  SDMLaunchProcess.h
//  libDaodan
//
//  Created by Sam Marshall on 10/30/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMLaunchProcess_h
#define Daodan_SDMLaunchProcess_h

#include "SDMHeader.h"

#define kLaunchPadPath "/Volumes/Data/Users/sam/Projects/Daodan/build/Release/Launchpad"
#define kDaodanPath "/Volumes/Data/Users/sam/Projects/Daodan/build/Release/libDaodan.dylib"

#define kDAODAN_RUN_ARCH "DAODAN_RUN_ARCH"
#define kDYLD_INSERT_LIBRARIES "DYLD_INSERT_LIBRARIES"

#define kLaunchPadKill "com.samdmarshall.Launchpad.Killswitch"

#define kiAmLaunchPad 0x647068636E75616C // "launchpd"

int acquireTaskForPortRight();
void spawnLaunchpad(int argc, const char *argv[]);
void launchNewProcess(int argc, const char *argv[]);
extern uint64_t iAmLaunchPad(int argc, const char *argv[]);
char* GenerateLaunchpadKillswitchName();

#endif
