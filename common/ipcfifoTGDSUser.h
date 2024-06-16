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
#ifndef __ipcfifoTGDSUser_h__
#define __ipcfifoTGDSUser_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "utilsTGDS.h"

struct sIPCSharedTGDSSpecific {
	uint32 frameCounter7;	//VBLANK counter7
	uint32 frameCounter9;	//VBLANK counter9
	
	//pocketspc 0.9
	uint8	PORT_SNES_TO_SPC[4];	//0x027FFFF8
	uint8	PORT_SPC_TO_SNES[4];	//0x027FFFFC
	uint32	APU_PROGRAM_COUNTER;	//0x27E0000	@APU PC
	uint32	APU_ADDR_CMD;	//SNEMUL_CMD / APU_ADDR_CMD ((volatile uint32*)(0x2800000-16))	//0x027FFFE8
	uint32	APU_ADDR_ANS;	//SNEMUL_ANS / ADDR_SNEMUL_ANS : //#define APU_ADDR_ANS ((volatile uint32*)(0x2800000-20))
	uint32	APU_ADDR_BLK;	//APU_ADDR_BLK / SNEMUL_BLK ((volatile uint32*)(0x2800000-24))
	volatile uint8 * APU_ADDR_BLKP;	//#define (vuint8*)APU_ADDR_BLKP == APU_ADDR_BLK
	uint32	APU_ADDR_CNT;	//#define APU_ADDR_CNT ((volatile uint32*)(0x2800000-60))	/ 0x27fffc4 // used a SNES SCanline counter, unused by snemulds
	
	u8 *rawSpcShared;
};

#define ALIGNED __attribute__ ((aligned(4)))

typedef sint16 int16;
#define UNION_CAST(x, destType) \
   (((union {__typeof__(x) a; destType b;})x).b)
   
#define VRAM_0        ((uint16*)0x6000000)
#define VRAM_A        ((uint16*)0x6800000)
#define VRAM_B        ((uint16*)0x6820000)
#define VRAM_C        ((uint16*)0x6840000)
//#define VRAM_D        ((uint16*)0x6860000)
#define VRAM_E        ((uint16*)0x6880000)
#define VRAM_F        ((uint16*)0x6890000)
#define VRAM_G        ((uint16*)0x6894000)
#define VRAM_H        ((uint16*)0x6898000)
#define VRAM_I        ((uint16*)0x68A0000)

//Seconds before sleepmode takes place
#define SLEEPMODE_SECONDS (int)(15)

//TGDS Memory Layout ARM7/ARM9 Cores
#define TGDS_ARM7_MALLOCSTART (u32)(0x06004000)
#define TGDS_ARM7_MALLOCSIZE (int)(4*1024)
#define TGDSDLDI_ARM7_ADDRESS (u32)(TGDS_ARM7_MALLOCSTART + TGDS_ARM7_MALLOCSIZE)
#define TGDS_ARM7_AUDIOBUFFER_STREAM (u32)(TGDSDLDI_ARM7_ADDRESS + (32*1024))

#define APU_RAM_ADDRESS     ((volatile unsigned char*) ((int)TGDS_ARM7_AUDIOBUFFER_STREAM) + (8*1024) )	//64K APU WORK RAM

#define APU_BRR_HASH_BUFFER	(volatile u32*)((int)0x023B8000)	//270K ~ worth of Hashed Samples from the APU core to remove stuttering : 0x02400000 - 0x48000 = 0x023B8000

#define FIFO_TGDSAUDIOPLAYER_DISABLEIRQ (u32)(0xFFAACC00)
#define FIFO_TGDSAUDIOPLAYER_ENABLEIRQ (u32)(0xFFAACC01)

#define POCKETSPC_ARM7COMMAND_STOP_SPC (u32)(0xFFAACC02)
#define POCKETSPC_ARM7COMMAND_LOAD_SPC (u32)(0xFFAACC03)


#define ISEMULATOR 1 //defined == TGDS Project does not self reload, undefined == TGDS Project self reloads

#define ARM7_PAYLOAD ((u32)0x023E8000)


#endif

#ifdef __cplusplus
extern "C" {
#endif

//NOT weak symbols : the implementation of these is project-defined (here)
extern void HandleFifoNotEmptyWeakRef(u32 cmd1, uint32 cmd2);
extern void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2);
extern void setupLibUtils();
extern struct sIPCSharedTGDSSpecific* getsIPCSharedTGDSSpecific();



//project specific stuff

#ifdef ARM9
extern void enableFastMode();
extern void disableFastMode();
#endif

extern uint32 ADDRPORT_SPC_TO_SNES;
extern uint32 ADDRPORT_SNES_TO_SPC;
extern uint32 ADDR_APU_PROGRAM_COUNTER;
extern uint32 ADDR_SNEMUL_CMD;				//APU_ADDR_CMD	//0x027FFFE8
extern uint32 ADDR_SNEMUL_ANS;				//APU_ADDR_ANS	//0x027fffec
extern uint32 ADDR_SNEMUL_BLK;				//APU_ADDR_BLK	//0x027fffe8

#ifdef __cplusplus
}
#endif