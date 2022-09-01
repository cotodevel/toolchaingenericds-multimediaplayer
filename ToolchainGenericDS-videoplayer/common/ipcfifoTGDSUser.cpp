
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
#include "pff.h"
#include "ima_adpcm.h"

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

//inherits what is defined in: ipcfifoTGDS.c
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmptyWeakRef(u32 cmd1, uint32 cmd2){
	switch (cmd1) {
		#ifdef ARM7
		case(FIFO_STOPSOUNDSTREAM_FILE):{
			backgroundMusicPlayer.stop();
		}
		break;
		
		case(FIFO_PLAYSOUNDSTREAM_FILE):{
			playSoundStreamARM7();
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
void playSound(u32 * buffer, int bufferSize, u32 flags, int ch){
	u32 cnt   = flags | SOUND_VOL(127) | SOUND_PAN(64) | (2 << 29) | (0 << 24); //(2=IMA-ADPCM
	int len = bufferSize;
	u16 freq = 32000;
	//writeARM7SoundChannelFromSource(ch, cnt, (u16)freq, (u32)buffer, (u32)len);
}

bool soundGameOverEmitted = false;
void gameoverSound(){
	//ARM7 ADPCM playback 
	char filename[256];
	strcpy(filename, "0:/ah.wav");
	char * filen = FS_getFileName(filename);
	strcat(filen, ".ima");
	u32 streamType = FIFO_PLAYSOUNDSTREAM_FILE;
	playSoundStreamFromFile((char*)&filen[2], false, streamType);
}

void MunchFoodSound(){
	//ARM7 ADPCM playback 
	char filename[256];
	strcpy(filename, "0:/munch.wav");
	char * filen = FS_getFileName(filename);
	strcat(filen, ".ima");
	u32 streamType = FIFO_PLAYSOUNDEFFECT_FILE;
	playSoundStreamFromFile((char*)&filen[2], false, streamType);
}

void BgMusic(char * filename){
	//ARM7 ADPCM playback 
	char * filen = FS_getFileName(filename);
	strcat(filen, ".ima");
	u32 streamType = FIFO_PLAYSOUNDSTREAM_FILE;
	playSoundStreamFromFile((char*)&filen[2], true, streamType);
}

bool bgMusicEnabled = false;
void BgMusicOff(){
	SendFIFOWords(FIFO_STOPSOUNDSTREAM_FILE, 0xFF);
}


void updateStreamCustomDecoder(u32 srcFrmt){

}

void freeSoundCustomDecoder(u32 srcFrmt){

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