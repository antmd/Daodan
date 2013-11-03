//
//  SDMObjcLexer.c
//  Daodan
//
//  Created by Sam Marshall on 11/3/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMObjcLexer_c
#define Daodan_SDMObjcLexer_c

#include "SDMObjcLexer.h"


#define kObjcTypeEncodingCount 0x15

static char* ObjcTypeEncoding[kObjcTypeEncodingCount] = {
	kObjcCharEncoding,
	kObjcIntEncoding,
	kObjcShortEncoding,
	kObjcLongEncoding,
	kObjcLLongEncoding,
	kObjcUCharEncoding,
	kObjcUIntEncoding,
	kObjcUShortEncoding,
	kObjcULongEncoding,
	kObjcULLongEncoding,
	kObjcFloatEncoding,
	kObjcDoubleEncoding,
	kObjcBoolEncoding,
	kObjcVoidEncoding,
	kObjcStringEncoding,
	kObjcIdEncoding,
	kObjcClassEncoding,
	kObjcSelEncoding,
	kObjcBitEncoding,
	kObjcPointerEncoding,
	kObjcUnknownEncoding
};

static char* ObjcTypeEncodingNames[kObjcTypeEncodingCount] = {
	"char",
	"int",
	"short",
	"long",
	"long long",
	"unsigned char",
	"unsigned int",
	"unsigned short",
	"unsigned long",
	"unsigned long long",
	"float",
	"double",
	"bool",
	"void",
	"char*",
	"id",
	"Class",
	":",
	"bitmask",
	"*",
	"UnknownType"
};

#define kObjcContainerTypeEncodingCount 0x1

char *ObjcContainerTypeEncodingNames[kObjcContainerTypeEncodingCount] = {
	"struct "
};

struct SDMSTRange SDMSTObjcGetTokenRangeFromOffset(char *type, uint64_t offset, char *token) {
	uint64_t counter = 0x0;
	while (strncmp(&(type[offset+counter]), token, strlen(token)) != 0x0) {
		counter++;
	}
	return SDMSTRangeMake((uintptr_t)offset, counter);
}

struct SDMSTRange SDMSTObjcGetStructContentsRange(char *type, uint64_t offset) {
	uint64_t stack = 0x1;
	uint64_t counter = 0x0;
	while (stack != 0x0) {
		if (strncmp(&(type[offset+counter]), kObjcStructTokenStart, sizeof(char)) == 0x0) {
			stack++;
		}
		if (strncmp(&(type[offset+counter]), kObjcStructTokenEnd, sizeof(char)) == 0x0) {
			stack--;
		}
		counter++;
	}
	counter--;
	return SDMSTRangeMake((uintptr_t)offset, counter);
}

struct SDMSTRange SDMSTObjcGetStructNameRange(char *contents, uint64_t offset) {
	return SDMSTObjcGetTokenRangeFromOffset(contents, offset, kObjcStructDefinitionToken);
}

struct SDMSTObjcType* SDMSTObjcDecodeType(char *type) {
	struct SDMSTObjcType *decode = calloc(0x1, sizeof(struct SDMSTObjcType));
	decode->token = calloc(0x1, sizeof(struct SDMSTObjcLexerToken));
	uint64_t length = strlen(type);
	if (length) {
		uint64_t offset = 0x0;
		while (offset < length) {
			uint32_t parsedLength = 0x1;
			uint32_t index = k32BitMask;
			for (uint32_t i = 0x0; i < kObjcTypeEncodingCount; i++) {
				if (strncmp(&(type[offset]), ObjcTypeEncoding[i], sizeof(char)) == 0x0) {
					index = i;
					break;
				}
			}
			if (index != k32BitMask) {
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				decode->token[decode->tokenCount].typeClass = SDMObjcLexerConvertIndexToToken(index);
				decode->token[decode->tokenCount].type = ObjcTypeEncodingNames[index];
				decode->token[decode->tokenCount].typeName = "";
				switch (decode->token[decode->tokenCount].typeClass) {
					case ObjcCharEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcIntEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcShortEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcLongEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcLLongEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcUCharEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcUIntEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcUShortEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcULongEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcULLongEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcFloatEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcDoubleEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcBoolEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcVoidEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcStringEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcIdEncoding: {
						uint64_t next = offset+0x1;
						if (strncmp(&(type[next]), kObjcNameTokenStart, sizeof(char)) == 0x0) {
							struct SDMSTRange nameRange = SDMSTObjcGetTokenRangeFromOffset(type, next+0x1, kObjcNameTokenEnd);
							char *name = calloc(0x1, sizeof(char)*(0x3+(uint32_t)nameRange.length));
							sprintf(name,"<");
							memcpy(&(name[0x1]), &(type[nameRange.offset]), sizeof(char)*nameRange.length);
							sprintf(name,"%s>",name);
							decode->token[decode->tokenCount].typeName = name;
							parsedLength += nameRange.length + 0x2;
							decode->tokenCount++;
						}
						break;
					};
					case ObjcClassEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcSelEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcBitEncoding: {
						decode->tokenCount++;
						break;
					};
					case ObjcPointerEncoding: {
						decode->token[decode->tokenCount].isPointer = true;
						break;
					};
					case ObjcUnknownEncoding: {
						decode->token[decode->tokenCount].typeName = "";
						decode->tokenCount++;
						break;
					};
					default: {
						break;
					};
				}
			} else {
				if (strncmp(&(type[offset]), kObjcPointerEncoding, sizeof(char)) == 0x0) {
					decode->token[decode->tokenCount].isPointer = true;
				}
				if (strncmp(&(type[offset]), kObjcUnknownEncoding, sizeof(char)) == 0x0) {
					decode->token[decode->tokenCount].typeName = "";
				}
				if (strncmp(&(type[offset]), kObjcStructTokenStart, sizeof(char)) == 0x0) {
					uint64_t next = offset+0x1;
					decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
					decode->token[decode->tokenCount].typeClass = ObjcStructEncoding;
					decode->token[decode->tokenCount].type = ObjcContainerTypeEncodingNames[0x0];
					struct SDMSTRange contentsRange = SDMSTObjcGetStructContentsRange(type, next);
					char *contents = calloc(0x1, sizeof(char)*((uint32_t)contentsRange.length+0x1));
					memcpy(contents, &(type[next]), contentsRange.length);
					struct SDMSTRange nameRange = SDMSTObjcGetStructNameRange(contents, 0x0);
					char *name = calloc(0x1, sizeof(char)*((uint32_t)nameRange.length+0x1));
					memcpy(name, &(contents[nameRange.offset]), nameRange.length);
					decode->token[decode->tokenCount].typeName = name;
					parsedLength += contentsRange.length + 0x1;

					// SDM: parse out children
					
					decode->tokenCount++;
				}
				if (strncmp(&(type[offset]), kObjcArrayTokenStart, sizeof(char)) == 0x0) {
					
				}
			}
			offset = offset + parsedLength;
		}
		
	}
	return decode;
}

#endif