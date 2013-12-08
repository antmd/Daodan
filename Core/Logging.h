//
//  Logging.h
//  Core
//
//  Created by Sam Marshall on 12/8/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Core_Logging_h
#define Core_Logging_h

#pragma mark -
#pragma mark Color Codes

#include <stdlib.h>

#define LoggerUseColorCodes (getenv("TERM") != 0x0) // SDM: Xcode console doesn't like these, but they work fine in a terminal

#define COLOR_NRM  "\x1B[0m"
#define COLOR_RED  "\x1B[31m"
#define COLOR_GRN  "\x1B[32m"
#define COLOR_YEL  "\x1B[33m"
#define COLOR_BLU  "\x1B[34m"
#define COLOR_MAG  "\x1B[35m"
#define COLOR_CYN  "\x1B[36m"
#define COLOR_WHT  "\x1B[37m"

#pragma mark -
#pragma mark Logging

#ifdef DEBUG
	#define DEBUG_LOGGER true
	#define RELEASE_LOGGER false
#else
	#define DEBUG_LOGGER false
	#define RELEASE_LOGGER true
#endif

enum LoggerPrintCodes {
	PrintCode_Norm = 0x0,
	PrintCode_OK = 0x1,
	PrintCode_TRY = 0x2,
	PrintCode_ERR = 0x3,
	PrintCode_NTR = 0x4,
	PrintCode_Count
};

static char* PrintColourCode[PrintCode_Count] = {
	COLOR_NRM,
	COLOR_GRN,
	COLOR_YEL,
	COLOR_RED,
	COLOR_BLU
};

static char* PrintCodeString[PrintCode_Count] = {
	"???",
	"OK!",
	"TRY",
	"ERR",
	"NTR"
};

#define LoggerPrintCodeColor(code) (LoggerUseColorCodes ? PrintColourCode[code] : "")
#define LoggerPrintCode(code) (PrintCodeString[code])

#define DPrint(code,...) \
	if (DEBUG_LOGGER) { \
		printf("[%s%s%s] ",LoggerPrintCodeColor(code),LoggerPrintCode(code),LoggerPrintCodeColor(PrintCode_Norm)); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	}
	
#define RPrint(code,...) \
	if (RELEASE_LOGGER) { \
		printf("[%s%s%s] ",LoggerPrintCodeColor(code),LoggerPrintCode(code),LoggerPrintCodeColor(PrintCode_Norm)); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	}
	
#ifdef DEBUG
#define LogPrint(code,...) DPrint(code,__VA_ARGS__)
#else
#define LogPrint(code,...) RPrint(code,__VA_ARGS__)
#endif


#endif
