/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

#ifndef __main7_h__
#define __main7_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "pff.h"
#include "soundTGDS.h"
#include "ima_adpcm.h"

struct addrList {
    u32 armRamAddress;
	u32 armEntryAddress;
    u32 armBootCodeOffsetInFile;
    s32 armBootCodeSize;
	u32 type;
};

#define TGDS_MB_V3_TYPE_ENTRYPOINT_ARM7 ((u32) 1)
#define TGDS_MB_V3_TYPE_ENTRYPOINT_ARM9 ((u32) 2)
#define TGDS_MB_V3_TYPE_DEFAULT_VALUE 	((u32) 3)

#define TGDS_MB_V3_ADDR_COUNT ((s32) 4)

#endif


#ifdef __cplusplus
extern "C" {
#endif

#if defined(ARM7VRAMCUSTOMCORE)

extern u8 * NDSHeaderStruct;
extern char debugBuf7[256];
extern struct addrList addresses[TGDS_MB_V3_ADDR_COUNT];
extern int compare(const void* a, const void* b);
extern int isNTROrTWLBinaryTGDSMB7(FATFS * currentFH, u8 * NDSHeaderStructInst, int NDSHeaderStructSize, u32 * ARM7i_HEADER_SCFG_EXT7Inst, bool * inIsTGDSTWLHomebrew);
extern void bootfile();
extern void stopBGMusic7();

//misc
extern char *itoa(int number, char *arr, int base);
extern void strrev(char *arr, int start, int end);

#endif

#ifdef __cplusplus
}
#endif

