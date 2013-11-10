//
//  SDMObjcRuntime.c
//  Daodan
//
//  Created by Sam Marshall on 11/10/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMObjcRuntime_c
#define Daodan_SDMObjcRuntime_c

#include "SDMObjcRuntime.h"

struct SDMSTObjcClass* SDMSTObjc1CreateClassFromProtocol(struct SDMSTObjc *objcData, struct SDMSTObjc1Protocol *prot) {
	struct SDMSTObjcClass *newClass = calloc(0x1, sizeof(struct SDMSTObjcClass));
	if (prot) {
		newClass->className = (char*)SDMSTCastSmallPointer(prot->name);
	}
	return newClass;
}

struct SDMSTObjcClass* SDMSTObjc1CreateClassFromCategory(struct SDMSTObjc *objcData, struct SDMSTObjc1Category *cat) {
	struct SDMSTObjcClass *newClass = calloc(0x1, sizeof(struct SDMSTObjcClass));
	if (cat) {
		newClass->className = (char*)SDMSTCastSmallPointer(cat->name);
	}
	return newClass;
}

struct SDMSTObjcClass* SDMSTObjc1CreateClassFromClass(struct SDMSTObjc *objcData, struct SDMSTObjc1Class *cls) {
	struct SDMSTObjcClass *newClass = calloc(0x1, sizeof(struct SDMSTObjcClass));
	if (cls) {
		if (cls->superClass != 0x0) {
			if (cls->superClass != cls->isa) {
				newClass->superCls = SDMSTObjc1CreateClassFromClass(objcData, (struct SDMSTObjc1Class *)SDMSTCastSmallPointer(cls->superClass));
			} else {
				newClass->superCls = (struct SDMSTObjcClass *)SDMSTCastSmallPointer(cls->superClass);
			}
			newClass->className = (char*)SDMSTCastSmallPointer(cls->name);
			
			struct SDMSTObjc1ClassIVarInfo *ivarInfo = ((struct SDMSTObjc1ClassIVarInfo *)SDMSTCastSmallPointer(cls->ivars));
			if (ivarInfo) {
				newClass->ivarCount = ivarInfo->count;
				newClass->ivar = calloc(newClass->ivarCount, sizeof(struct SDMSTObjcIVar));
				struct SDMSTObjc1ClassIVar *ivarOffset = (struct SDMSTObjc1ClassIVar *)((uint64_t)ivarInfo + (uint64_t)sizeof(struct SDMSTObjc1ClassIVarInfo));
				for (uint32_t i = 0x0; i < newClass->ivarCount; i++) {
					newClass->ivar[i].name = (char*)SDMSTCastSmallPointer(ivarOffset[i].name);
					newClass->ivar[i].type = (char*)SDMSTCastSmallPointer(ivarOffset[i].type);
					newClass->ivar[i].offset = (uintptr_t)(ivarOffset[i].offset);
				}
				
			}
			
			struct SDMSTObjc1ClassMethodInfo *methodInfo = ((struct SDMSTObjc1ClassMethodInfo *)SDMSTCastSmallPointer(cls->methods));
			if (methodInfo && (((uint64_t)methodInfo >= objcData->classRange.offset && (uint64_t)methodInfo < ( (uint64_t)(objcData->clsMRange.offset) + (uint64_t)objcData->clsMRange.length)) || ((uint64_t)methodInfo >= objcData->instMRange.offset && (uint64_t)methodInfo < ((uint64_t)(objcData->instMRange.offset) + (uint64_t)objcData->instMRange.length)))) {
				newClass->methodCount = methodInfo->count;
				newClass->method = calloc(newClass->methodCount, sizeof(struct SDMSTObjcMethod));
				struct SDMSTObjc1ClassMethod *methodOffset = (struct SDMSTObjc1ClassMethod *)((uint64_t)methodInfo + (uint64_t)sizeof(struct SDMSTObjc1ClassMethodInfo));
				for (uint32_t i = 0x0; i < newClass->methodCount; i++) {
					newClass->method[i].name = (char*)SDMSTCastSmallPointer(methodOffset[i].name);
					newClass->method[i].type = (char*)SDMSTCastSmallPointer(methodOffset[i].type);
					newClass->method[i].offset = (uintptr_t)(methodOffset[i].imp);
				}
			}
			
			struct SDMSTObjc1Protocol *protocolInfo = ((struct SDMSTObjc1Protocol *)SDMSTCastSmallPointer(cls->protocols));
			if (protocolInfo) {
				
			}
		}
	}
	return newClass;
}

void SDMSTObjc1CreateClassFromSymbol(struct SDMSTObjc *objcData, struct SDMSTObjc1Symtab *symtab) {
	if (symtab) {
		uint32_t counter = symtab->catCount + symtab->classCount;
		struct SDMSTObjc1SymtabDefinition *symbol = (struct SDMSTObjc1SymtabDefinition *)((uint64_t)(symtab) + (uint64_t)sizeof(struct SDMSTObjc1Symtab));
		for (uint32_t i = 0x0; i < counter; i++) {
			if ((symbol[i].defintion >= objcData->classRange.offset) && (symbol[i].defintion <= ((uint64_t)(objcData->classRange.offset) + (uint64_t)objcData->classRange.length))) {
				objcData->cls = realloc(objcData->cls, sizeof(struct SDMSTObjcClass)*(objcData->clsCount+0x1));
				struct SDMSTObjcClass *newClass = SDMSTObjc1CreateClassFromClass(objcData, ((struct SDMSTObjc1Class *)SDMSTCastSmallPointer(symbol[i].defintion)));
				memcpy(&(objcData->cls[objcData->clsCount]), newClass, sizeof(struct SDMSTObjcClass));
				free(newClass);
				objcData->clsCount++;
			}
			if ((symbol[i].defintion >= objcData->catRange.offset) && (symbol[i].defintion <= ((uint64_t)(objcData->catRange.offset) + (uint64_t)objcData->catRange.length))) {
				objcData->cls = realloc(objcData->cls, sizeof(struct SDMSTObjcClass)*(objcData->clsCount+0x1));
				struct SDMSTObjcClass *newClass = SDMSTObjc1CreateClassFromCategory(objcData, ((struct SDMSTObjc1Category *)SDMSTCastSmallPointer(symbol[i].defintion)));
				memcpy(&(objcData->cls[objcData->clsCount]), newClass, sizeof(struct SDMSTObjcClass));
				free(newClass);
				objcData->clsCount++;
			}
			symbol = (struct SDMSTObjc1SymtabDefinition *)((uint64_t)symbol + (uint64_t)sizeof(struct SDMSTObjc1SymtabDefinition));
		}
	}
}

struct SDMSTObjcClass* SDMSTObjc2ClassCreateFromClass(struct SDMSTObjc2Class *cls, struct SDMSTObjc2Class *parentClass, struct SDMSTRange dataRange) {
	struct SDMSTObjcClass *newClass = calloc(0x1, sizeof(struct SDMSTObjcClass));
	if (cls != parentClass) {
		if (((uint64_t)(cls->isa) >= dataRange.offset) && ((uint64_t)(cls->isa) < (dataRange.offset + dataRange.length))) {
			newClass->superCls = SDMSTObjc2ClassCreateFromClass((cls->isa),cls, dataRange);
			newClass->className = (char*)((struct SDMSTObjc2ClassData *)(cls->data)->name);
			
			struct SDMSTObjc2ClassIVarInfo *ivarInfo = ((struct SDMSTObjc2ClassIVarInfo*)(cls->data->ivar));
			if (ivarInfo) {
				newClass->ivarCount = ivarInfo->count;
				newClass->ivar = calloc(newClass->ivarCount, sizeof(struct SDMSTObjcIVar));
				struct SDMSTObjc2ClassIVar *ivarOffset = (struct SDMSTObjc2ClassIVar *)((uint64_t)ivarInfo + (uint64_t)sizeof(struct SDMSTObjc2ClassIVarInfo));
				for (uint32_t i = 0x0; i < newClass->ivarCount; i++) {
					newClass->ivar[i].name = (char*)(ivarOffset[i].name);
					newClass->ivar[i].type = (char*)(ivarOffset[i].type);
					newClass->ivar[i].offset = (uintptr_t)(ivarOffset[i].offset);
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