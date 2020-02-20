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

//TGDS required version: IPC Version: 1.3

//IPC FIFO Description: 
//		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 														// Access to TGDS internal IPC FIFO structure. 		(ipcfifoTGDS.h)
//		struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;		// Access to TGDS Project (User) IPC FIFO structure	(ipcfifoTGDSUser.h)


//inherits what is defined in: ipcfifoTGDS.h
#ifndef __specific_shared_h__
#define __specific_shared_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "dswnifi.h"
#include "utilsTGDS.h"

#define VRAM_D		((s16*)0x06000000)
#define SIWRAM		((s16*)0x037F8000)

#define ARM9COMMAND_SUCCESS (uint32)(0xFFFFFF01)
#define ARM9COMMAND_UPDATE_BUFFER (uint32)(0xFFFFFF02)
#define ARM9COMMAND_TOUCHDOWN (uint32)(0xFFFFFF04)
#define ARM9COMMAND_TOUCHMOVE (uint32)(0xFFFFFF05)
#define ARM9COMMAND_TOUCHUP (uint32)(0xFFFFFF06)

#define ARM7STATE_IDLE (uint32)(0xFFFFFF07)
#define ARM7STATE_WAITING (uint32)(0xFFFFFF08)
#define ARM7STATE_WAITCOPY (uint32)(0xFFFFFF09)

#define ARM7COMMAND_START_SOUND (uint32)(0xFFFFFF10)
#define ARM7COMMAND_STOP_SOUND (uint32)(0xFFFFFF11)
#define ARM7COMMAND_SOUND_SETMULT (uint32)(0xffff03A1)
#define ARM7COMMAND_SOUND_SETRATE (uint32)(0xffff03A2)
#define ARM7COMMAND_SOUND_SETLEN (uint32)(0xffff03A3)
#define ARM7COMMAND_SOUND_COPY (uint32)(0xFFFFFF15)
#define ARM7COMMAND_SOUND_DEINTERLACE (uint32)(0xFFFFFF16)
#define ARM7COMMAND_BOOT_GBAMP (uint32)(0xFFFFFF19)
#define ARM7COMMAND_BOOT_SUPERCARD (uint32)(0xFFFFFF20)
#define ARM7COMMAND_BOOT_MIGHTYMAX (uint32)(0xFFFFFF21)
#define ARM7COMMAND_BOOT_CHISHM (uint32)(0xFFFFFF22)
#define ARM7COMMAND_PSG_COMMAND (uint32)(0xFFFFFF23)
#define ARM7COMMAND_SAVE_WIFI (uint32)(0xFFFFFF24)
#define ARM7COMMAND_LOAD_WIFI (uint32)(0xFFFFFF25)
#define ARM7COMMAND_RESET_BACKLIGHT (uint32)(0xFFFFFF26)
#define ARM7COMMAND_DISABLE_SLEEPMODE (uint32)(0xFFFFFF27)
#define ARM7COMMAND_ENABLE_SLEEPMODE (uint32)(0xFFFFFF28)

#define BIT(n) (1 << (n))

struct sSoundPlayerStruct {
	s16 *arm9L;
	s16 *arm9R;
	
	s16 *interlaced;
	int channels;
	u8 volume;
	
	//u32 tX;
	//u32 tY;
	
	int psgChannel;
	u32 cr;
	u32 timer;
	//Mic bits end
} __attribute__((aligned (4)));

//---------------------------------------------------------------------------------
struct sIPCSharedTGDSSpecific {
//---------------------------------------------------------------------------------
	uint32 frameCounter7;	//VBLANK counter7
	uint32 frameCounter9;	//VBLANK counter9
	struct sSoundPlayerStruct sndPlayerCtx;
};

//types used by DSOrganize
typedef sint16 int16;
#define UNION_CAST(x, destType) \
   (((union {__typeof__(x) a; destType b;})x).b)
   
#define VRAM_0        ((uint16*)0x6000000)
#define VRAM_A        ((uint16*)0x6800000)
#define VRAM_B        ((uint16*)0x6820000)
//#define VRAM_C        ((uint16*)0x6840000)
//#define VRAM_D        ((uint16*)0x6860000)
#define VRAM_E        ((uint16*)0x6880000)
#define VRAM_F        ((uint16*)0x6890000)
#define VRAM_G        ((uint16*)0x6894000)
#define VRAM_H        ((uint16*)0x6898000)
#define VRAM_I        ((uint16*)0x68A0000)

//Seconds before sleepmode takes place
#define SLEEPMODE_SECONDS (int)(15)

#ifdef ARM9
//Used by ARM9. Required internally by ARM7
#define TGDSDLDI_ARM7_ADDRESS (u32)(0x06000000 + (96*1024))
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

//NOT weak symbols : the implementation of these is project-defined (here)
extern void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2);
extern void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2);

#ifdef __cplusplus
}
#endif