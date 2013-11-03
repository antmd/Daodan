//
//  SDMObjcRuntime1.h
//  Daodan
//
//  Created by Sam Marshall on 11/2/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMObjcRuntime1_h
#define Daodan_SDMObjcRuntime1_h

#define kObjcCatClsMeth "__cat_cls_meth"
#define kObjcCatInstMeth "__cat_inst_meth"
#define kObjcStringObject "__string_object"
#define kObjcCStringObject "__cstring_object"
#define kObjcMessageRefs "__message_refs"
#define kObjcSelFixup "__sel_fixup"
#define kObjcClsRefs "__cls_refs"
#define kObjcClass "__class"
#define kObjcMetaClass "__meta_class"
#define kObjcClsMeth "__cls_meth"
#define kObjcInstMeth "__inst_meth"
#define kObjcProtocol "__protocol"
#define kObjcCategory "__category"
#define kObjcClassVars "__class_vars"
#define kObjcInstanceVars "__instance_vars"
#define kObjcModuleInfo "__module_info"
#define kObjcSymbols "__symbols"

struct SDMSTObjc1ClassMethodDescInfo {
	uint32_t count;
} ATR_PACK SDMSTObjc1ClassMethodDescInfo;

struct SDMSTObjc1ClassMethodInfo {
	uint32_t entrySize;
	uint32_t count;
} ATR_PACK SDMSTObjc1ClassMethodInfo;

struct SDMSTObjc1ClassIVarInfo {
	uint32_t count;
} ATR_PACK SDMSTObjc1ClassIVarInfo;

struct SDMSTObjc1ClassIVar {
	uint32_t name;
	uint32_t type;
	uint32_t offset;
} ATR_PACK SDMSTObjc1ClassIVar;

struct SDMSTObjc1ClassMethod {
	uint32_t name;
	uint32_t type;
	uint32_t imp;
} ATR_PACK SDMSTObjc1ClassMethod;

struct SDMSTObjc1Protocol {
	uint32_t isa;
	uint32_t name;
	uint32_t protocolList;
	uint32_t instanceMethodDesc;
	uint32_t classMethodDesc;
} ATR_PACK SDMSTObjc1Protocol;

struct SDMSTObjc1Category {
	uint32_t name;
	uint32_t className;
	uint32_t instanceMethods;
	uint32_t classMethods;
	uint32_t protocols;
} ATR_PACK SDMSTObjc1Category;

struct SDMSTObjc1Class {
	uint32_t isa;
	uint32_t superClass;
	uint32_t name;
	uint32_t version;
	uint32_t info;
	uint32_t instanceSize;
	uint32_t ivars;
	uint32_t methods;
	uint32_t cache;
	uint32_t protocols;
} ATR_PACK SDMSTObjc1Class;

struct SDMSTObjc1SymtabDefinition {
	uint32_t defintion;
} ATR_PACK SDMSTObjc1SymtabDefinition;

struct SDMSTObjc1Symtab {
	uint32_t selectorRefCount;
	uint32_t refs;
	uint16_t classCount;
	uint16_t catCount;
} ATR_PACK SDMSTObjc1Symtab;

#endif
