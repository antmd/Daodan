/*
 *  SDMHeader.h
 *
 *  Copyright (c) 2013, Sam Marshall
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
 *  	This product includes software developed by the Sam Marshall.
 *  4. Neither the name of the Sam Marshall nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY Sam Marshall ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Sam Marshall BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef Daodan_SDMHeader_h
#define Daodan_SDMHeader_h

#pragma mark -
#pragma mark Includes
#pragma mark Standard Libs
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <notify.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma mark -
#pragma mark Color Codes

#define UseColorCodes false // SDM: Xcode console doesn't like these, but they work fine in a terminal
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

#ifndef DEBUG
	#define DEBUG_LOGGER true
#else
	#define DEBUG_LOGGER false
#endif
#define DEFAULT_LOGGER false

enum SDMPrintCodes {
	PrintCode_OK = 0x1,
	PrintCode_TRY = 0x2,
	PrintCode_ERR = 0x3,
	PrintCode_NTR = 0x4,
	PrintCode_ProgName = 0x5,
	PrintCode_LogLib = 0x6,
	PrintCode_Norm = 0x7
};

#define SDMPrintCodeColor(code) (UseColorCodes ? (code == PrintCode_OK ? COLOR_GRN : (code == PrintCode_TRY ? COLOR_YEL : (code == PrintCode_ERR ? COLOR_RED : (code == PrintCode_NTR ? COLOR_MAG : (code == PrintCode_ProgName ? COLOR_CYN : (code == PrintCode_LogLib ? COLOR_BLU : COLOR_NRM)))))) : "")
#define SDMPrintCode(code) (code == PrintCode_OK ? "OK!" : (code == PrintCode_TRY ? "TRY" : (code == PrintCode_ERR ? "ERR" : (code == PrintCode_NTR ? "NTR" : (code == PrintCode_ProgName ? getprogname() : (code == PrintCode_LogLib ? LoggingLibraryName : (code == PrintCode_Norm ? "" : "???")))))))

#define LoggingLibrary true
#ifdef LoggingLibrary
#define LoggingLibraryName "Daodan"
#endif

#define SDMFormatPrint(silent,code,...) \
	if (silent) { \
		printf(""); \
	} else { \
		printf("[%s%s%s]",SDMPrintCodeColor(PrintCode_ProgName),SDMPrintCode(PrintCode_ProgName),SDMPrintCodeColor(PrintCode_Norm)); \
		if (LoggingLibrary) { \
		printf("[%s%s%s]",SDMPrintCodeColor(PrintCode_LogLib),SDMPrintCode(PrintCode_LogLib),SDMPrintCodeColor(PrintCode_Norm)); \
		} \
		printf("[%s%s%s] ",SDMPrintCodeColor(code),SDMPrintCode(code),SDMPrintCodeColor(PrintCode_Norm)); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	}

#define DPrint(code,...) SDMFormatPrint(false,code,__VA_ARGS__)
#define RPrint(code,...) SDMFormatPrint(true,code,__VA_ARGS__)

#ifdef DEBUG
#define SDMPrint(code,...) DPrint(code,__VA_ARGS__)
#else
#define SDMPrint(code,...) RPrint(code,__VA_ARGS__)
#endif

#pragma mark -
#pragma mark Attributes
#define ATR_PACK __attribute__ ((packed))
#define ATR_FUNC(name) __attribute__ ((ifunc(name)))


#pragma mark -
#pragma mark Types

struct SDMSTRange {
	uintptr_t offset;
	uint64_t length;
} ATR_PACK SDMSTRange;

#pragma mark -
#pragma mark Pointers

#include "Pointer.h"

#define SDMSwapEndian16(num) ((num<<8) | (num>>8))
#define SDMSwapEndian32(num) (((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) |((num<<24)&0xff000000))

#pragma mark -
#pragma mark Functions

#define SDMGetNumberOfDigits(a) (a > 0 ? (int)log10(a)+1 : 1)
#define SDMSTRangeMake(a,b) ((struct SDMSTRange){a, b})
bool makeNewFolderAt(char *path, mode_t mode);

#endif