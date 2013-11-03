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
	char *name;
	char *type;
	uintptr_t offset;
} ATR_PACK SDMSTObjcMethod;

struct SDMSTObjcProtocol {
	uintptr_t offset;
} ATR_PACK SDMSTObjcProtocol;

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
			printf("class: %s\n",newClass->className);
			struct SDMSTObjc2ClassIVarInfo *ivarInfo = ((struct SDMSTObjc2ClassIVarInfo*)(cls->data->ivar));
			if (ivarInfo) {
				newClass->ivarCount = ivarInfo->count;
				newClass->ivar = calloc(newClass->ivarCount, sizeof(struct SDMSTObjcIVar));
				struct SDMSTObjc2ClassIVar *ivarOffset = (struct SDMSTObjc2ClassIVar *)((uint64_t)ivarInfo + (uint64_t)sizeof(struct SDMSTObjc2ClassIVarInfo));
				for (uint32_t i = 0x0; i < newClass->ivarCount; i++) {
					newClass->ivar[i].name = (char*)(ivarOffset[i].name);
					newClass->ivar[i].type = (char*)(ivarOffset[i].type);
					newClass->ivar[i].offset = (uintptr_t)(ivarOffset[i].offset);
					printf("\t%s\n",newClass->ivar[i].name);
				}
			}
			
			struct SDMSTObjc2ClassMethodInfo *methodInfo = ((struct SDMSTObjc2ClassMethodInfo*)(cls->data->method));
			if (methodInfo) {
				newClass->methodCount = methodInfo->count;
				newClass->method = calloc(newClass->methodCount, sizeof(struct SDMSTObjcMethod));
				struct SDMSTObjc2ClassMethod *methodOffset = (struct SDMSTObjc2ClassMethod *)((uint64_t)methodInfo + (uint64_t)sizeof(struct SDMSTObjc2ClassMethodInfo));
				for (uint32_t i = 0x0; i < newClass->methodCount; i++) {
					newClass->method[i].name = (char*)(methodOffset[i].name);
					newClass->method[i].type = (char*)(methodOffset[i].type);
					newClass->method[i].offset = (uintptr_t)(methodOffset[i].imp);
				}
			}
			
			struct SDMSTObjc2ClassProtcolInfo *protocolInfo = ((struct SDMSTObjc2ClassProtcolInfo*)(cls->data->protocol));
			if (protocolInfo) {
				newClass->protocolCount = (uint32_t)(protocolInfo->count);
				newClass->protocol = calloc(newClass->protocolCount, sizeof(struct SDMSTObjcProtocol));
				struct SDMSTObjc2ClassProtocol *protocolOffset = (struct SDMSTObjc2ClassProtocol *)((uint64_t)protocolInfo + (uint64_t)sizeof(struct SDMSTObjc2ClassProtcolInfo));
				for (uint32_t i = 0x0; i < newClass->protocolCount; i++) {
					newClass->protocol[i].offset = (uintptr_t)(protocolOffset[i].offset);
				}
			}
		}
	}
	return newClass;
}

#endif
