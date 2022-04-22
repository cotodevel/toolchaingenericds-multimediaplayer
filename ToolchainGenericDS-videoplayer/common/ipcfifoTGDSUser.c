
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

#include "ipcfifoTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "libutilsShared.h"
#include "microphoneShared.h"

#ifdef ARM7
#include <string.h>

#include "main.h"
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "biosTGDS.h"

#endif

#ifdef ARM9

#include <stdbool.h>
#include "main.h"
#include "wifi_arm9.h"
#include "dswnifi_lib.h"
#endif

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
struct sIPCSharedTGDSSpecific* getsIPCSharedTGDSSpecific(){
	struct sIPCSharedTGDSSpecific* sIPCSharedTGDSSpecificInst = (struct sIPCSharedTGDSSpecific*)(TGDSIPCUserStartAddress);
	return sIPCSharedTGDSSpecificInst;
}

//inherits what is defined in: ipcfifoTGDS.c
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmptyWeakRef(u32 cmd1, uint32 cmd2){
	switch (cmd1) {
		#ifdef ARM7
		case(FIFO_TGDSVIDEOPLAYER_STOPSOUND):{
			playerStopARM7();
		}
		break;
		
		case(FIFO_DIRECTVIDEOFRAME_SETUP):{
			handleARM7FSSetup();
		}
		break;
		
		case(FIFO_DIRECTVIDEOFRAME_RENDER):{
			handleARM7FSRender();
		}
		break;
		
		case(FIFO_TGDSAUDIOPLAYER_ENABLEIRQ):{
			REG_DISPSTAT = (DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
			REG_IE = REG_IE | (IRQ_VBLANK|IRQ_VCOUNT);
		}
		break;
		
		case(FIFO_TGDSAUDIOPLAYER_DISABLEIRQ):{
			REG_DISPSTAT = 0;
			REG_IE = REG_IE & ~(IRQ_VBLANK|IRQ_VCOUNT);
		}
		break;
		#endif
		
		#ifdef ARM9
		
		#endif
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2){
}

//project specific stuff

#ifdef ARM9

void updateStreamCustomDecoder(u32 srcFrmt){

}

void freeSoundCustomDecoder(u32 srcFrmt){

}

#endif

#ifdef ARM9
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u32 setupDirectVideoFrameRender(struct fd * videoStructFD, char * videoStructFDFilename){
	struct sIPCSharedTGDSSpecific* sharedIPC = getsIPCSharedTGDSSpecific();
	char * filename = (char*)&sharedIPC->filename[0];
	strcpy(filename, videoStructFDFilename);
	
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	fifomsg[33] = (uint32)0xFFFFCCAA;
	SendFIFOWords(FIFO_DIRECTVIDEOFRAME_SETUP, 0xFF);
	while(fifomsg[33] == (uint32)0xFFFFCCAA){
		swiDelay(1);
	}
	return fifomsg[33];
}

#ifdef ARM9
__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
struct videoFrame * readVideoFrameByDirectARM7(int fileOffset, int bufferSize){
	return (struct videoFrame *)0;
}
#endif

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void enableFastMode(){
	SendFIFOWords(FIFO_TGDSAUDIOPLAYER_DISABLEIRQ, 0xFF);
	
	REG_DISPSTAT = DISP_VBLANK_IRQ;
	REG_IE = REG_IE & ~(IRQ_VCOUNT);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void disableFastMode(){
	SendFIFOWords(FIFO_TGDSAUDIOPLAYER_ENABLEIRQ, 0xFF);
	
	REG_DISPSTAT = (DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
	REG_IE = REG_IE | (IRQ_VCOUNT);
}

#endif

//Libutils setup: TGDS project uses Soundstream, WIFI, ARM7 malloc, etc.
void setupLibUtils(){
	//libutils:
	
	//Stage 0
	#ifdef ARM9
	initializeLibUtils9(
		NULL, //ARM7 & ARM9
		NULL, //ARM9 
		NULL, //ARM9: bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType)
		NULL,  //ARM9: void updateStream() 
		NULL, //ARM7 & ARM9: DeInitWIFI()
		NULL //ARM9: bool switch_dswnifi_mode(sint32 mode)
	);
	#endif
	
	//Stage 1
	#ifdef ARM7
	initializeLibUtils7(
		NULL, //ARM7 & ARM9
		NULL, //ARM7
		NULL, //ARM7
		NULL, //ARM7: void TIMER1Handler()
		NULL, //ARM7: void stopSound()
		NULL, //ARM7: void setupSound()
		(initMallocARM7LibUtils_fn)&initARM7Malloc, //ARM7: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 ARM7MallocSize);
		NULL, //ARM7 & ARM9: DeInitWIFI()
		NULL  //ARM7: micInterrupt()
	);
	#endif
}