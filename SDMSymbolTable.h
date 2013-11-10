/*
 *  SDMSymbolTable.h
 *  SDMSymbolTable
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

#ifndef _SDMSYMBOLTABLE_H_
#define _SDMSYMBOLTABLE_H_

#pragma mark -
#pragma mark Includes
#include "SDMHeader.h"
#include "SDMObjcRuntime.h"
#pragma mark sys/*
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/errno.h>
#pragma mark mach/*
#include <mach/mach.h>
#pragma mark mach-o/*
#include <mach-o/ldsyms.h>
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>
#include <mach-o/fat.h>
#pragma mark other
#include <fcntl.h>
#include <dlfcn.h>

#pragma mark -
#pragma mark Types

struct SDMSTFunctionCommand {
	struct load_command loadCmd;
	uint32_t offset;
	uint32_t size;
} ATR_PACK SDMSTFunctionCommand;

struct SDMSTSymbolTableListEntry {
	union {
		uint32_t n_strx;
	} n_un;
	uint8_t n_type;
	uint8_t n_sect;
	uint16_t n_desc;
} ATR_PACK SDMSTSymbolTableListEntry;

struct SDMSTSeg32Data {
	uint32_t vmaddr;
	uint32_t vmsize;
	uint32_t fileoff;
} ATR_PACK SDMSTSeg32Data;

struct SDMSTSeg64Data {
	uint64_t vmaddr;
	uint64_t vmsize;
	uint64_t fileoff;
} ATR_PACK SDMSTSeg64Data;

typedef uintptr_t* (*SDMSTFunctionCall)();

struct SDMSTFunction {
	char *name;
	SDMSTFunctionCall offset;
} ATR_PACK SDMSTFunction;

struct SDMSTSegmentEntry {
	uint32_t cmd;
	uint32_t cmdsize;
	char segname[0x10];
} ATR_PACK SDMSTSegmentEntry;

struct SDMSTLibraryArchitecture {
	cpu_type_t type;
	cpu_subtype_t subtype;
} ATR_PACK SDMSTLibraryArchitecture;

struct SDMSTLibraryTableInfo {
	uint32_t imageNumber;
	uintptr_t *mhOffset;
	struct SDMSTSegmentEntry *textSeg;
	struct SDMSTSegmentEntry *linkSeg;
	struct SDMSTSegmentEntry *objcSeg;
	struct symtab_command *symtabCommands;
	uint32_t symtabCount;
	struct SDMSTFunctionCommand *functCmd;
	uint32_t headerMagic;
	bool is64bit;
	struct SDMSTLibraryArchitecture arch;
} ATR_PACK SDMSTLibraryTableInfo;

struct SDMSTMachOSymbol {
	uint32_t tableNumber;
	uint32_t symbolNumber;
	uintptr_t offset;
	char *name;
	bool isStub;
} ATR_PACK SDMSTMachOSymbol;

struct SDMSTSubroutine {
	uintptr_t offset;
	char *name;
	uintptr_t sectionOffset;
} ATR_PACK SDMSTSubroute;

struct SDMSTDependency {
	uintptr_t loadCmd;
	struct dylib_command dyl;
} ATR_PACK SDMSTDependency;

struct SDMSTLibrary {
	bool couldLoad;
	char *libraryPath;
	uintptr_t* libraryHandle;
	uint64_t librarySize;
	struct SDMSTLibraryTableInfo *libInfo;
	struct SDMSTDependency *dependency;
	uint32_t dependencyCount;
	struct SDMSTMachOSymbol *table;
	uint32_t symbolCount;
	struct SDMSTSubroutine *subroutine;
	uint32_t subroutineCount;
	struct SDMSTObjc *objcInfo;
} ATR_PACK SDMSTLibrary;

struct SDMSTBinary {
	uintptr_t *arch;
	uint32_t archCount;
} ATR_PACK SDMSTBinary;

#pragma mark -
#pragma mark Declarations

struct SDMSTLibrary* SDMSTLoadLibrary(char *path, uint32_t index, bool silent);
struct SDMSTFunction* SDMSTCreateFunction(struct SDMSTLibrary *libTable, char *name);
struct SDMSTRange SDMSTRangeOfSubroutine(struct SDMSTSubroutine *subroutine, struct SDMSTLibrary *libTable);
SDMSTFunctionCall SDMSTSymbolLookup(struct SDMSTLibrary *libTable, char *symbolName);
void SDMSTFunctionRelease(struct SDMSTFunction *function);
void SDMSTLibraryRelease(struct SDMSTLibrary *libTable);
struct SDMSTLibraryArchitecture SDMSTGetArchitecture();

#endif