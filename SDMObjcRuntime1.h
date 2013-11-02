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

struct SDMSTObjc1ClassIVar {
	uint32_t name;
	uint32_t type;
	uint32_t offset;
} ATR_PACK SDMSTObjc1ClassIVar;

#endif
