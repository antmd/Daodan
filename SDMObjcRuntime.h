//
//  SDMObjcRuntime.h
//  Daodan
//
//  Created by Sam Marshall on 11/2/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMObjcRuntime_h
#define Daodan_SDMObjcRuntime_h
#include <mach-o/dyld.h>
#include "SDMObjcRuntime1.h"
#include "SDMObjcRuntime2.h"

struct SDMSTObjcIVar {
	char *name;
	char *type;
	uintptr_t offset;
} ATR_PACK SDMSTObjcIVar;

struct SDMSTObjcMethod {
	uintptr_t name;
	uintptr_t type;
	uintptr_t offset;
} ATR_PACK SDMSTObjcMethod;

struct SDMSTObjcClass {
	struct SDMSTObjcClass *superCls;
	char *className;
	struct SDMSTObjcIVar *ivar;
	uint32_t ivarCount;
	struct SDMSTObjcMethod *method;
	uint32_t methodCount;
	struct SDMSTObjcProtocol *protocol;
	uint32_t protocolCount;
} ATR_PACK SDMSTObjcClass;

struct SDMSTObjcModule {
	char *impName;
	struct SDMSTObjcClass *symbol;
} ATR_PACK SDMSTObjcModule;

struct SDMSTObjcModuleRaw {
	uint32_t version;
	uint32_t size;
	uint32_t name;
	uint32_t symtab;
} ATR_PACK SDMSTObjcModuleRaw;

struct SDMSTObjc {
	struct SDMSTObjcClass *cls;
	uint32_t clsCount;
} ATR_PACK SDMSTObjc;

inline struct SDMSTObjcClass* SDMSTObjc2ClassCreateFromClass(struct SDMSTObjc2Class *cls, struct SDMSTObjc2Class *parentClass, struct SDMSTRange dataRange) {
	struct SDMSTObjcClass *newClass = calloc(0x1, sizeof(struct SDMSTObjcClass));
	if (cls != parentClass) {
		if (((uint64_t)(cls->isa) >= dataRange.offset) && ((uint64_t)(cls->isa) < (dataRange.offset + dataRange.length))) {
			newClass->superCls = SDMSTObjc2ClassCreateFromClass((cls->isa),cls, dataRange);
			newClass->className = (char*)((struct SDMSTObjc2ClassData *)(cls->data)->name);
			
			struct SDMSTObjc2ClassIVarInfo *ivarInfo = ((struct SDMSTObjc2ClassIVarInfo*)(cls->data->ivar));
			if (ivarInfo) {
				newClass->ivarCount = ivarInfo->count;
				for (uint32_t i = 0x0; i < newClass->ivarCount; i++) {
					
				}
			}
			
			struct SDMSTObjc2ClassMethodInfo *methodInfo = ((struct SDMSTObjc2ClassMethodInfo*)(cls->data->method));
			if (methodInfo) {
				newClass->methodCount = methodInfo->count;
				for (uint32_t i = 0x0; i < newClass->methodCount; i++) {
					
				}
			}
			
			struct SDMSTObjc2ClassProtcolInfo *protocolInfo = ((struct SDMSTObjc2ClassProtcolInfo*)(cls->data->protocol));
			if (protocolInfo) {
				newClass->protocolCount = (uint32_t)(protocolInfo->count);
				for (uint32_t i = 0x0; i < newClass->protocolCount; i++) {
					
				}
			}
		}
	}
	return newClass;
}

#endif
