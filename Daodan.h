/*
 *  Daodan.h
 *  Daodan
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

#ifndef Daodan_Daodan_h
#define Daodan_Daodan_h

#include "SDMHeader.h"
#include "SDMSymbolTable.h"

enum DAODAN_QUEUE {
	DAODAN_MACH_SEND = 0x1,
	DAODAN_MACH_RECV = 0x2,
};

typedef struct SDMDaodanBundle {
	char path[0x400];
} ATR_PACK SDMDaodanBundle;

typedef struct DaodanMachMessage {
	mach_msg_header_t header;
	char data[0x400];
} ATR_PACK DaodanMachMessage;

void initDaodan();
void unloadDaodan();
uintptr_t daodanLookupFunction(char *name);
void daodanLoadSymbolTableForImage(uint32_t index);
uint32_t SDMGetIndexForLibraryPath(char *path);
uint32_t SDMGetExecuteImage();
uint32_t SDMGetImageLocation(const struct mach_header *mh, char **path);
void SDMAddImageHook(const struct mach_header* mh, intptr_t vmaddr_slide);
void SDMRemoveImageHook(const struct mach_header* mh, intptr_t vmaddr_slide);

#endif
