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
//		TGDSIPC 		= 	Access to TGDS internal IPC FIFO structure. 		(ipcfifoTGDS.h)
//		TGDSUSERIPC		=	Access to TGDS Project (User) IPC FIFO structure	(ipcfifoTGDSUser.h)

//inherits what is defined in: ipcfifoTGDS.h
#ifndef __ipcfifoTGDSUser_h__
#define __ipcfifoTGDSUser_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "dswnifi.h"
#include "utilsTGDS.h"

#define VRAM_D		((s16*)0x06000000)
#define SIWRAM		((s16*)0x037F8000)

#define ARM7COMMAND_PSG_COMMAND (uint32)(0xFFFFFF23)
#define ARM7COMMAND_SOUND_DEINTERLACE (uint32)(0xFFFFFF16)

#define BIT(n) (1 << (n))

//---------------------------------------------------------------------------------
typedef struct sIPCSharedTGDSSpecific{
//---------------------------------------------------------------------------------
	uint32 frameCounter7;	//VBLANK counter7
	uint32 frameCounter9;	//VBLANK counter9
}  IPCSharedTGDSSpecific	__attribute__((aligned (4)));

#define TGDSUSERIPC ((IPCSharedTGDSSpecific volatile *)(0x027FF000 + TGDSIPCSize))

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