//
//  SDMObjcRuntime.h
//  Daodan
//
//  Created by Sam Marshall on 11/2/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMObjcRuntime_h
#define Daodan_SDMObjcRuntime_h
#include "SDMObjcLexer.h"
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

struct SDMSTObjcModuleContainer {
	struct SDMSTObjcModule *module;
	uint32_t moduleCount;
} ATR_PACK SDMSTObjcModuleContainer;

struct SDMSTObjc {
	struct SDMSTObjcClass *cls;
	uint32_t clsCount;
	struct SDMSTRange classRange;
	struct SDMSTRange catRange;
	struct SDMSTRange protRange;
	struct SDMSTRange clsMRange;
	struct SDMSTRange instMRange;
} ATR_PACK SDMSTObjc;

struct SDMSTObjcClass* SDMSTObjc1CreateClassFromProtocol(struct SDMSTObjc *objcData, struct SDMSTObjc1Protocol *prot);
struct SDMSTObjcClass* SDMSTObjc1CreateClassFromCategory(struct SDMSTObjc *objcData, struct SDMSTObjc1Category *cat);
struct SDMSTObjcClass* SDMSTObjc1CreateClassFromClass(struct SDMSTObjc *objcData, struct SDMSTObjc1Class *cls);
void SDMSTObjc1CreateClassFromSymbol(struct SDMSTObjc *objcData, struct SDMSTObjc1Symtab *symtab);
struct SDMSTObjcClass* SDMSTObjc2ClassCreateFromClass(struct SDMSTObjc2Class *cls, struct SDMSTObjc2Class *parentClass, struct SDMSTRange dataRange);

#endif
