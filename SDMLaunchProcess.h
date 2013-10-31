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

int acquireTaskForPortRight();
void spawnFromLaunchpad(int argc, const char *argv[]);
void launchNewProcess(int argc, const char *argv[]);
extern uint64_t iAmLaunchPad(int argc, const char *argv[]);

#endif
