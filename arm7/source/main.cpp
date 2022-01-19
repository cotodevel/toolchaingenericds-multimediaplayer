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
#include "ipcfifoTGDSUser.h"
#include "soundTGDS.h"
#include "biosTGDS.h"
#include "timerTGDS.h"
#include "xmem.h"
#include "spc.h"

FATFS Fatfs;					// Petit-FatFs work area 
//struct TGDSVideoFrameContext videoCtx;
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
	//memset(&videoCtx, 0, sizeof(struct TGDSVideoFrameContext));
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
		//strcpy((char*)0x02000000, "File open ERR!");
	}
	else{
		//strcpy((char*)0x02000000, "File open OK!");
	}


	pf_lseek(0);
	//pf_read((u8*)&videoCtx, sizeof(struct TGDSVideoFrameContext), &br);		/* Load a page data */
	
	
	//decode SNES audio here
	uint8_t *spcfile = NULL;
	u32 spcLength = pf_size();
	
	spcfile = (uint8_t *)TGDSARM7Malloc(spcLength);
	pf_read((u8*)spcfile, spcLength, &br);
	
	if(!spcInit(spcfile, spcLength))
	{
		strcpy((char*)0x02000000, "Fail parsing SPC!");
		TGDSARM7Free(spcfile);
		while(1==1){}
	}

	//strcpy((char*)0x02000000, "SPC parse OK!");
	//while(1==1){}

	multRate = 1;
	sndRate = SPC_FREQ;
	sampleLen = SPC_OUT_SIZE;
	
	//ARM7 sound code
	setupSoundTGDSVideoPlayerARM7();
	strpcmL0 = (s16*)lBufferARM7;	//VRAM_D;
	strpcmL1 = (s16*)(lBufferARM7 + (SPC_OUT_SIZE >> 2));	//strpcmL0 + (size >> 1);
	strpcmR0 = (s16*)rBufferARM7;	//strpcmL1 + (size >> 1);
	strpcmR1 = (s16*)(rBufferARM7 + (SPC_OUT_SIZE >> 2));	//strpcmR0 + (size >> 1);
	
	spcDecode();
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setupSoundTGDSVideoPlayerARM7() {
	//Init SoundSampleContext
	initSound();

	sndCursor = 1;
	multRate = 1;
	
	//mallocData(sampleLen * 2 * multRate);
    
	TIMERXDATA(1) = TIMER_FREQ((sndRate * multRate *2));
	TIMERXCNT(1) = TIMER_DIV_1 | TIMER_ENABLE;
  
	TIMERXDATA(2) =  (0x10000) - (sampleLen*2);
	TIMERXCNT(2) = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
	
	//irqSet(IRQ_TIMER1, TIMER1Handler);
	int ch;
	
	for(ch=0;ch<4;++ch)
	{
		SCHANNEL_CR(ch) = 0;
		SCHANNEL_TIMER(ch) = SOUND_FREQ((sndRate * multRate));
		SCHANNEL_LENGTH(ch) = (sampleLen * multRate) >> 1;
		SCHANNEL_REPEAT_POINT(ch) = 0;
	}

	//irqSet(IRQ_VBLANK, 0);
	//irqDisable(IRQ_VBLANK);
	REG_IE&=~IRQ_VBLANK;
	REG_IE |= IRQ_TIMER2;
	
	lastL = 0;
	lastR = 0;
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
	/*			TGDS 1.6 Standard ARM7 Init code start	*/
	//wait for VRAM Block to be assigned from ARM9->ARM7 (ARM7 has load/store on byte/half/words on VRAM)
	while (!(*((vuint8*)0x04000240) & (1<<0))); //VRAM C ARM7
	while (!(*((vuint8*)0x04000240) & (1<<1))); //VRAM D ARM7
	
	/*			TGDS 1.6 Standard ARM7 Init code end	*/
	REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	
	while (1) {
		handleARM7SVC();	/* Do not remove, handles TGDS services */
		//IRQWait(0, IRQ_VBLANK | IRQ_VCOUNT | IRQ_IPCSYNC | IRQ_RECVFIFO_NOT_EMPTY);
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
	//player.stop();
	pf_lseek(0);
	
	REG_IE&=~IRQ_TIMER2;
	
	TIMERXDATA(1) = 0;
	TIMERXCNT(1) = 0;
	TIMERXDATA(2) = 0;
	TIMERXCNT(2) = 0;
}
