//
//  SDMDaodanInjection_Template.h
//  Daodan
//
//  Created by Sam Marshall on 11/10/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMDaodanInjection_Template_h
#define Daodan_SDMDaodanInjection_Template_h

#include "SDMHeader.h"

struct SDMDaodanInjectable {
	Pointer replacement;
	Pointer replacee;
} ATR_PACK SDMDaodanInjectable;

struct SDMDaodanInject {
	char *libraryPath;
	char *symbolName;
	Pointer offset;
	struct SDMDaodanInjectable inject;
} ATR_PACK SDMDaodanInject;

#endif
