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

#define kObjcTypeEncodingNameCount 0x12

char* ObjcTypeEncoding[kObjcTypeEncodingNameCount] = {
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
	kObjcSelEncoding
};

char* ObjcTypeEncodingNames[kObjcTypeEncodingNameCount] = {
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
	":"
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
			for (uint32_t i = 0x0; i < kObjcTypeEncodingNameCount; i++) {
				if (strncmp(&(type[offset]), ObjcTypeEncoding[i], sizeof(char)) == 0x0) {
					index = i;
					break;
				}
			}
			if (index != k32BitMask) {
				//printf("found type %s\n",ObjcTypeEncodingNames[index]);
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				decode->token[decode->tokenCount].typeClass = SDMObjcLexerConvertIndexToken(index);
				decode->token[decode->tokenCount].type = ObjcTypeEncodingNames[index];
				switch (decode->token[decode->tokenCount].typeClass) {
					case ObjcCharEncoding: {
						break;
					};
					case ObjcIntEncoding: {
						break;
					};
					case ObjcShortEncoding: {
						break;
					};
					case ObjcLongEncoding: {
						break;
					};
					case ObjcLLongEncoding: {
						break;
					};
					case ObjcUCharEncoding: {
						break;
					};
					case ObjcUIntEncoding: {
						break;
					};
					case ObjcUShortEncoding: {
						break;
					};
					case ObjcULongEncoding: {
						break;
					};
					case ObjcULLongEncoding: {
						break;
					};
					case ObjcFloatEncoding: {
						break;
					};
					case ObjcDoubleEncoding: {
						break;
					};
					case ObjcBoolEncoding: {
						break;
					};
					case ObjcVoidEncoding: {
						break;
					};
					case ObjcStringEncoding: {
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
						}
						break;
					};
					case ObjcClassEncoding: {
						break;
					};
					case ObjcSelEncoding: {
						break;
					};
					case ObjcBitEncoding: {
						break;
					};
					case ObjcPointerEncoding: {
						break;
					};
					case ObjcUnknownEncoding: {
						break;
					};
					default: {
						break;
					};
				}
				decode->tokenCount++;
			} else {
				if (strncmp(&(type[offset]), kObjcStructTokenStart, sizeof(char)) == 0x0) {
					uint64_t next = offset+0x1;
					decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
					decode->token[decode->tokenCount].typeClass = ObjcStructEncoding;
					decode->token[decode->tokenCount].type = "struct ";
					struct SDMSTRange contentsRange = SDMSTObjcGetStructContentsRange(type, next);
					char *contents = calloc(0x1, sizeof(char)*((uint32_t)contentsRange.length+0x1));
					memcpy(contents, &(type[next]), contentsRange.length);
					struct SDMSTRange nameRange = SDMSTObjcGetStructNameRange(contents, 0x0);
					char *name = calloc(0x1, sizeof(char)*((uint32_t)nameRange.length+0x1));
					memcpy(name, &(contents[nameRange.offset]), nameRange.length);
					decode->token[decode->tokenCount].typeName = name;
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