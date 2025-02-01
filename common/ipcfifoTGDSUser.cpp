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
#include "TGDS_threads.h"

#ifdef ARM7
#include <string.h>
#include "main.h"
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "spitscTGDS.h"

#ifdef ARM7SPCCUSTOMCORE
#include "apu.h"
#include "dsp.h"
#include "spcdefs.h"
#endif

#if defined(ARM7VRAMCUSTOMCORE)
#include "pff.h"
#include "ima_adpcm.h"
#endif

#endif

#ifdef ARM9

#include <stdbool.h>
#include "main.h"
#include "wifi_arm9.h"
#include "dswnifi_lib.h"
#include "soundTGDS.h"
#include "biosTGDS.h"
#endif

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
struct sIPCSharedTGDSSpecific* getsIPCSharedTGDSSpecific(){
	struct sIPCSharedTGDSSpecific* sIPCSharedTGDSSpecificInst = (struct sIPCSharedTGDSSpecific*)(TGDSIPCUserStartAddress);
	return sIPCSharedTGDSSpecificInst;
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void HandleFifoNotEmptyWeakRef(u32 cmd1, uint32 cmd2){
	switch (cmd1) {
		//NDS7: 
		#ifdef ARM7
		
		//void disableFastMode();
		case(FIFO_TGDSAUDIOPLAYER_ENABLEIRQ):{
			REG_DISPSTAT = (DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
			REG_IE = REG_IE | (IRQ_VBLANK|IRQ_VCOUNT);
			enableARM7TouchScreen();
			uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
			setValueSafe(&fifomsg[34], (uint32)0);
		}
		break;
		
		//void enableFastMode();
		case(FIFO_TGDSAUDIOPLAYER_DISABLEIRQ):{
			REG_DISPSTAT = 0;
			REG_IE = REG_IE & ~(IRQ_VBLANK|IRQ_VCOUNT);
			disableARM7TouchScreen();
			uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
			setValueSafe(&fifomsg[34], (uint32)0);
		}
		break;
		
		case(FIFO_STOP_ARM7_VRAM_CORE):{
			#ifdef ARM7SPCCUSTOMCORE
			StopSoundSPC();
			#endif
			
			#if defined(ARM7VRAMCUSTOMCORE)
			playerStopARM7();
			#endif
			uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
			setValueSafe(&fifomsg[34], (uint32)0);
		}
		break;

		#if defined(ARM7VRAMCUSTOMCORE)
		case(FIFO_STOPSOUNDSTREAM_FILE):{
			stopBGMusic7();
		}
		break;
		
		case(FIFO_PLAYSOUNDSTREAM_FILE):{
			playSoundStreamARM7();
		}
		break;
		#endif
		
		//pocketspc 0.9
		#ifdef ARM7SPCCUSTOMCORE
		case POCKETSPC_ARM7COMMAND_STOP_SPC:{
			StopSoundSPC();
		}break;
		case POCKETSPC_ARM7COMMAND_LOAD_SPC:{
			REG_DISPSTAT = 0;
			REG_IE = REG_IE & ~(IRQ_VBLANK|IRQ_VCOUNT|IRQ_HBLANK);
			
			struct sIPCSharedTGDSSpecific* sharedIPC = getsIPCSharedTGDSSpecific();
			LoadSpc(sharedIPC->rawSpcShared);
			SetupSoundSPC();
		}break;	
		#endif
		
		#endif
		
		//NDS9: 
		#ifdef ARM9
		
		#endif
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2){
}

//Libutils setup: TGDS project uses Soundstream, WIFI, ARM7 malloc, etc.
void setupLibUtils(){
	//libutils:
	
	//Stage 0
	#if defined(ARM7VRAMCUSTOMCORE)
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
			NULL, //ARM7: micInterrupt()
			NULL, //ARM7: DeInitWIFI()
			NULL  //ARM7: void wifiAddressHandler( void * address, void * userdata )
		);
		#endif
	#endif
	
	#ifdef ARM7SPCCUSTOMCORE
		//Stage 0
		#ifdef ARM9
		initializeLibUtils9(
			(HandleFifoNotEmptyWeakRefLibUtils_fn)&libUtilsFIFONotEmpty, //ARM7 & ARM9
			(timerWifiInterruptARM9LibUtils_fn)&Timer_50ms, //ARM9 
			(SoundStreamStopSoundStreamARM9LibUtils_fn)&stopSoundStream,	//ARM9: bool stopSoundStream(struct fd * tgdsStructFD1, struct fd * tgdsStructFD2, int * internalCodecType)
			(SoundStreamUpdateSoundStreamARM9LibUtils_fn)&updateStream, //ARM9: void updateStream() 
			(wifiDeinitARM7ARM9LibUtils_fn)&DeInitWIFI, //ARM7 & ARM9: DeInitWIFI()
			(wifiswitchDsWnifiModeARM9LibUtils_fn)&switch_dswnifi_mode //ARM9: bool switch_dswnifi_mode(sint32 mode)
		);
		#endif
		
		//Stage 1: ARM7 Stage1 & SPC core
		#if defined(ARM7) 
		initializeLibUtils7(
			NULL, //ARM7 & ARM9
			NULL, //ARM7
			NULL,  //ARM7
			(SoundStreamTimerHandlerARM7LibUtils_fn)&TIMER1Handler, //ARM7: void TIMER1Handler()
			(SoundStreamStopSoundARM7LibUtils_fn)&stopSound, 	//ARM7: void stopSound()
			(SoundStreamSetupSoundARM7LibUtils_fn)&setupSound,	//ARM7: void setupSound()
			NULL, //ARM7: void initARM7Malloc(u32 ARM7MallocStartaddress, u32 ARM7MallocSize);
			NULL,  //ARM7 & ARM9: DeInitWIFI()
			NULL, //ARM7: micInterrupt()
			NULL, //ARM7: DeInitWIFI()
			NULL//ARM7: void wifiAddressHandler( void * address, void * userdata )
		);
		#endif
	#endif
}

//project specific stuff

#ifdef ARM9

void BgMusic(char * filename){
	//ARM7 ADPCM playback 
	char * filen = FS_getFileName(filename);
	strcat(filen, ".ima");
	u32 streamType = FIFO_PLAYSOUNDSTREAM_FILE;
	playSoundStreamFromFile((char*)&filen[2], true, streamType);
}

void BgMusicOff(){
	SendFIFOWords(FIFO_STOPSOUNDSTREAM_FILE, 0xFF);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void haltARM7(){
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[34], (uint32)FIFO_STOP_ARM7_VRAM_CORE);
	SendFIFOWords(FIFO_STOP_ARM7_VRAM_CORE, 0xFF);
	while( getValueSafe(&fifomsg[34]) == (uint32)FIFO_STOP_ARM7_VRAM_CORE){
		swiDelay(1);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
u32 playSoundStreamFromFile(char * videoStructFDFilename, bool loop, u32 streamType){
	struct sIPCSharedTGDSSpecific* sharedIPC = getsIPCSharedTGDSSpecific();
	char * filename = (char*)&sharedIPC->filename[0];
	strcpy(filename, videoStructFDFilename);
	
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	fifomsg[33] = (uint32)0xFFFFCCAA;
	fifomsg[34] = (uint32)loop;
	fifomsg[35] = (uint32)streamType;
	SendFIFOWords(FIFO_PLAYSOUNDSTREAM_FILE, 0xFF);
	while(fifomsg[33] == (uint32)0xFFFFCCAA){
		swiDelay(1);
	}
	return fifomsg[33];
}

void enableFastMode(){
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[34], (uint32)FIFO_TGDSAUDIOPLAYER_DISABLEIRQ);
	SendFIFOWords(FIFO_TGDSAUDIOPLAYER_DISABLEIRQ, 0xFF);
	while( getValueSafe(&fifomsg[34]) == (uint32)FIFO_TGDSAUDIOPLAYER_DISABLEIRQ){
		swiDelay(1);
	}
}

void disableFastMode(){
	uint32 * fifomsg = (uint32 *)NDS_UNCACHED_SCRATCHPAD;
	setValueSafe(&fifomsg[34], (uint32)FIFO_TGDSAUDIOPLAYER_ENABLEIRQ);
	SendFIFOWords(FIFO_TGDSAUDIOPLAYER_ENABLEIRQ, 0xFF);
	while( getValueSafe(&fifomsg[34]) == (uint32)FIFO_TGDSAUDIOPLAYER_ENABLEIRQ){
		swiDelay(1);
	}
}
#endif

uint32 ADDRPORT_SPC_TO_SNES=0;
uint32 ADDRPORT_SNES_TO_SPC=0;
uint32 ADDR_APU_PROGRAM_COUNTER=0;
uint32 ADDR_SNEMUL_CMD=0;				//APU_ADDR_CMD	//0x027FFFE8
uint32 ADDR_SNEMUL_ANS=0;				//APU_ADDR_ANS	//0x027fffec
uint32 ADDR_SNEMUL_BLK=0;				//APU_ADDR_BLK	//0x027fffe8

//APU Ports from SnemulDS properly binded with Assembly APU Core
void update_spc_ports(){
	struct sIPCSharedTGDSSpecific* sharedIPC = getsIPCSharedTGDSSpecific();
	ADDRPORT_SPC_TO_SNES	=	(uint32)(uint8*)&sharedIPC->PORT_SPC_TO_SNES[0];
	ADDRPORT_SNES_TO_SPC	=	(uint32)(uint8*)&sharedIPC->PORT_SNES_TO_SPC[0]; 
	ADDR_APU_PROGRAM_COUNTER=	(uint32)(volatile uint32*)&sharedIPC->APU_PROGRAM_COUNTER;	//0x27E0000	@APU PC
	
	ADDR_SNEMUL_CMD	=	(uint32)(volatile uint32*)&sharedIPC->APU_ADDR_CMD;	//0x027FFFE8	// SNEMUL_CMD
	ADDR_SNEMUL_ANS	=	(uint32)(volatile uint32*)&sharedIPC->APU_ADDR_ANS;	//0x027fffec	// SNEMUL_ANS
	ADDR_SNEMUL_BLK	=	(uint32)(volatile uint32*)&sharedIPC->APU_ADDR_BLK;	//0x027fffe8	// SNEMUL_BLK
	sharedIPC->APU_ADDR_BLKP = (uint8 *)ADDR_SNEMUL_BLK;
}
