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

#include "main.h"
#include "biosTGDS.h"
#include "loader.h"
#include "busTGDS.h"
#include "dmaTGDS.h"
#include "spifwTGDS.h"
#include "wifi_arm7.h"
#include "pff.h"
#include "TGDSVideo.h"
#include "ipcfifoTGDSUser.h"
#include "ima_adpcm.h"
#include "soundTGDS.h"
#include "biosTGDS.h"
#include "timerTGDS.h"

FATFS Fatfs;					// Petit-FatFs work area 
struct TGDSVideoFrameContext videoCtx;
struct soundPlayerContext soundData;
char fname[256];

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleARM7FSSetup(){
	uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;			
	memset(&videoCtx, 0, sizeof(struct TGDSVideoFrameContext));
	UINT br;	/* Bytes read */
	uint8_t fresult;
	
	fresult = pf_mount(&Fatfs);	/* Initialize file system */
	if (fresult != FR_OK) { /* File System could not be mounted */
		fifomsg[33] = 0xAABBCCDD;
	}
	struct sIPCSharedTGDSSpecific* sharedIPC = getsIPCSharedTGDSSpecific();
	char * filename = (char*)&sharedIPC->filename[0];
	strcpy((char*)fname, filename);
	fresult = pf_open(fname);
	
	int argBuffer[MAXPRINT7ARGVCOUNT];
	memset((unsigned char *)&argBuffer[0], 0, sizeof(argBuffer));
	argBuffer[0] = 0xc070ffff;
	
	if (fresult != FR_OK) { /* File System could not be mounted */
		//strcpy((char*)0x02000000, "File open ERR:");
		//strcat((char*)0x02000000, filename);
		swiDelay(4000);
	}
	else{
		//strcpy((char*)0x02000000, "File open OK!");
		//strcat((char*)0x02000000, filename);
	}
	pf_lseek(0);
	//pf_read((u8*)&videoCtx, sizeof(struct TGDSVideoFrameContext), &br);		/* Load a page data */
	
	
	//decode audio here
	bool loop_audio = false;
	bool automatic_updates = false;
	if(player.play(loop_audio, automatic_updates, ADPCM_SIZE, stopSoundStreamUser) == 0){
		//ADPCM Playback!
	}
	
	fifomsg[33] = (u32)fresult;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleARM7FSRender(){
	uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;
	int fileOffset = (int)fifomsg[32];
	int bufferSize = (int)fifomsg[33];
	fifomsg[34] = (u32)0;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int argc, char **argv)  {
	REG_IE|=(IRQ_VBLANK|IRQ_HBLANK); //X button depends on this
	while (1) {
		handleARM7SVC();	/* Do not remove, handles TGDS services */
		IRQWait(0, IRQ_HBLANK);
	}
	return 0;
}

bool stopSoundStreamUser(){

}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void playerStopARM7(){
	memset((void *)strpcmL0, 0, 512);
	memset((void *)strpcmL1, 0, 512);
	memset((void *)strpcmR0, 0, 512);
	memset((void *)strpcmR1, 0, 512);

	REG_IE&=~IRQ_TIMER2;
	
	TIMERXDATA(1) = 0;
	TIMERXCNT(1) = 0;
	TIMERXDATA(2) = 0;
	TIMERXCNT(2) = 0;
	for(int ch=0;ch<4;++ch)
	{
		SCHANNEL_CR(ch) = 0;
		SCHANNEL_TIMER(ch) = 0;
		SCHANNEL_LENGTH(ch) = 0;
		SCHANNEL_REPEAT_POINT(ch) = 0;
	}
}
