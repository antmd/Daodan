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

static char *ObjcContainerTypeEncodingNames[kObjcContainerTypeEncodingCount] = {
	"struct"
};

#define kObjcStackSizeCount 0xa

static char *ObjcStackSize[kObjcStackSizeCount] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9"
};

struct SDMSTRange SDMSTObjcStackSize(char *type, uint64_t offset, uint64_t *stackSize) {
	uint64_t counter = 0x0;
	bool findStackSize = true;
	while (findStackSize) {
		findStackSize = false;
		for (uint32_t i = 0x0; i < kObjcStackSizeCount; i++) {
			if (strncmp(&(type[offset+counter]), ObjcStackSize[i], sizeof(char)) == 0x0) {
				counter++;
				findStackSize = true;
				break;
			}
		}
	}
	struct SDMSTRange stackRange = SDMSTRangeMake((uintptr_t)offset, counter);
	char *stack = calloc((uint32_t)stackRange.length+0x1, sizeof(char));
	memcpy(stack, &(type[offset]), (uint32_t)stackRange.length);
	*stackSize = atoi(stack);
	free(stack);
	return stackRange;
}

char* SDMSTObjcPointersForToken(struct SDMSTObjcLexerToken *token) {
	char *pointers = calloc(token->pointerCount+0x1, sizeof(char));
	for (uint32_t i = 0x0; i < token->pointerCount; i++) {
		sprintf(pointers,"%s*",pointers);
	}
	return pointers;
}

struct SDMSTRange SDMSTObjcGetTokenRangeFromOffset(char *type, uint64_t offset, char *token) {
	uint64_t counter = 0x0;
	while (strncmp(&(type[offset+counter]), token, strlen(token)) != 0x0) {
		counter++;
	}
	return SDMSTRangeMake((uintptr_t)offset, counter);
}

char* SDMSTObjcCreateMethodDescription(struct SDMSTObjcType *type, char *name) {
	uint32_t nameLength = 0x1;
	if (name) {
		nameLength += strlen(name);
	} else {
		name = "";
	}
	char *description = calloc(0x1, sizeof(char)*(nameLength+0x3+strlen(type->token[0x0].type)));
	uint32_t counter = 0x0;
	uint32_t argCount = 0x0;
	for (uint32_t i = counter+0x3; i < type->tokenCount; i++) {
		argCount++;
	}
	if (counter != argCount) {
		sprintf(description,"(%s)",type->token[0x0].type);
		uint32_t offset = 0x0;
		while (counter < argCount) {
			if (offset) {
				description = realloc(description, sizeof(char)*(strlen(description)+0x2));
				sprintf(description,"%s ",description);
			}
			struct SDMSTRange methodArgRange = SDMSTObjcGetTokenRangeFromOffset(name, offset, kObjcSelEncoding);
			char *argName = calloc(0x1, sizeof(char)*((uint32_t)methodArgRange.length+0x1));
			memcpy(argName, &(name[offset]), (uint32_t)methodArgRange.length);
			uint32_t formatLength = (uint32_t)(0x8+strlen(argName)+strlen(type->token[counter+0x3].type)+SDMGetNumberOfDigits(counter));
			char *formatName = calloc(0x1, sizeof(char)*formatLength);
			sprintf(formatName,"%s:(%s)_arg%01i",argName,type->token[counter+0x3].type,counter);
			description = realloc(description, sizeof(char)*(strlen(description)+formatLength));
			memcpy(&(description[strlen(description)]), formatName, formatLength);
			free(formatName);
			free(argName);
			offset = offset + (uint32_t)methodArgRange.length + 0x1;
			counter++;
		}
	} else {
		sprintf(description,"(%s)%s",type->token[0x0].type,name);
	}
	description = realloc(description, sizeof(char)*(strlen(description)+0x2));
	sprintf(description,"%s;",description);
	return description;
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
						}
						decode->tokenCount++;
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
						decode->token[decode->tokenCount].pointerCount++;
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
				uint64_t stackSize;
				struct SDMSTRange stackRange = SDMSTObjcStackSize(type, offset, &stackSize);
				if (stackRange.length) {
					parsedLength = (uint32_t)stackRange.length;
				} else {
					if (strncmp(&(type[offset]), kObjcPointerEncoding, sizeof(char)) == 0x0) {
						decode->token[decode->tokenCount].pointerCount++;
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
			}
			offset = offset + parsedLength;
		}
		
	}
	return decode;
}

#endif