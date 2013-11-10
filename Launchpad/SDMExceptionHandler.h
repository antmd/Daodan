//
//  SDMExceptionHandler.h
//  Daodan
//
//  Created by Sam Marshall on 11/10/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMExceptionHandler_h
#define Daodan_SDMExceptionHandler_h

#include "SDMHeader.h"
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <mach/mach_vm.h>
#include <mach/mach.h>

void SDMDaodanSetupExceptionHandler();

#endif
