//
//  SDMDaodanPayload.h
//  Daodan
//
//  Created by Sam Marshall on 11/10/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMDaodanPayload_h
#define Daodan_SDMDaodanPayload_h

#include "SDMHeader.h"
#include "SDMDaodanInjection_Template.h"
#include <CoreFoundation/CoreFoundation.h>

struct SDMDaodanPayload {
	struct SDMDaodanInject *injectable;
	uint32_t symbolCount;
} ATR_PACK SDMDaodanPayload;

#endif
