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
#include <sys/types.h>
#include <sys/stat.h>

#pragma mark -
#pragma mark Types

#ifndef DEBUG
	#define DEBUG_LOGGER TRUE
#else
	#define DEBUG_LOGGER FALSE
#endif

#define DEFAULT_LOGGER FALSE

#define kiAmLaunchPad 0x647068636E75616C // "launchpd"

#define ATR_PACK __attribute__ ((packed))

enum SDMPrintCodes {
	PrintCode_OK = 0x1,
	PrintCode_TRY = 0x2,
	PrintCode_ERR = 0x3,
	PrintCode_NTR = 0x4
};

struct SDMSTRange {
	uintptr_t offset;
	uint64_t length;
} ATR_PACK SDMSTRange;

#pragma mark -
#pragma mark Color Codes

#define UseColorCodes true // SDM: Xcode console doesn't like these, but they work fine in a terminal

#define COLOR_NRM  "\x1B[0m"
#define COLOR_RED  "\x1B[31m"
#define COLOR_GRN  "\x1B[32m"
#define COLOR_YEL  "\x1B[33m"
#define COLOR_BLU  "\x1B[34m"
#define COLOR_MAG  "\x1B[35m"
#define COLOR_CYN  "\x1B[36m"
#define COLOR_WHT  "\x1B[37m"

#define SDMPrintCodeColor(code) (UseColorCodes ? (code == PrintCode_OK ? COLOR_GRN : (code == PrintCode_TRY ? COLOR_YEL : (code == PrintCode_ERR ? COLOR_RED : (code == PrintCode_NTR ? COLOR_MAG : COLOR_BLU)))) : "")
#define SDMPrintCode(code) (code == PrintCode_OK ? "OK!" : (code == PrintCode_TRY ? "TRY" : (code == PrintCode_ERR ? "ERR" : (code == PrintCode_NTR ? "NTR" : "???"))))

#define SDMPrint(silent,code,...) if (silent) {\
		printf(""); \
	} else {\
		printf("[%s%s%s][%sDaodan%s][%s%s%s] ",COLOR_CYN,getprogname(),COLOR_NRM,COLOR_BLU,COLOR_NRM,SDMPrintCodeColor(code),SDMPrintCode(code),(UseColorCodes ? COLOR_NRM : "")); printf(__VA_ARGS__); printf("\n"); \
	}


#define FWRITE_STRING_TO_FILE(a,b) fwrite(a, sizeof(char), strlen(a), b)

#define SDMSTRangeMake(a,b) ((struct SDMSTRange){a, b})

inline bool makeNewFolderAt(char *path, mode_t mode) {
	bool result = false;
	struct stat st;
	if (stat(path, &st) == -1) {
		int mkdirResult = mkdir(path, mode);
		result = (mkdirResult == 0 ? true : false);
	}
	return result;
}

#endif