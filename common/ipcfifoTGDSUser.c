
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

#include "ipcfifoTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "soundplayerShared.h"
#include "timerTGDS.h"

#ifdef ARM7
#include <string.h>
#include "interrupts.h"
#include "main.h"
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "click_raw.h"
#include "soundTGDS.h"
#endif

#ifdef ARM9

#include <stdbool.h>
#include "main.h"
#include "wifi_arm9.h"
#include "sound.h"

#endif

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmptyWeakRef(uint32 data0, uint32 data1){
	
	switch (data0) {
		//NDS7: 
		#ifdef ARM7
		case ARM7COMMAND_SOUND_DEINTERLACE:
		{
			s16 *lbuf = NULL;
			s16 *rbuf = NULL;
			
			if(!sndCursor)
			{
				lbuf = strpcmL0;
				rbuf = strpcmR0;
			}
			else
			{
				lbuf = strpcmL1;
				rbuf = strpcmR1;
			}
			struct soundPlayerContext * soundPlayerCtx = soundIPC();
			s16 *iSrc = soundPlayerCtx->interlaced;
			u32 i = 0;
			int vMul = soundPlayerCtx->volume;
			int lSample = 0;
			int rSample = 0;
			
			switch(multRate)
			{
				case 1:{
					if(soundPlayerCtx->channels == 2)
					{
						for(i=0;i<sampleLen;++i)
						{					
							lSample = *iSrc++;
							rSample = *iSrc++;
							
							*lbuf++ = checkClipping((lSample * vMul) >> 2);
							*rbuf++ = checkClipping((rSample * vMul) >> 2);
						}
					}
					else
					{
						for(i=0;i<sampleLen;++i)
						{					
							lSample = *iSrc++;
							
							lSample = checkClipping((lSample * vMul) >> 2);
							
							*lbuf++ = lSample;
							*rbuf++ = lSample;
						}
					}
				}	
				break;
				case 2:{
					for(i=0;i<sampleLen;++i)
					{					
						if(soundPlayerCtx->channels == 2)
						{
							lSample = *iSrc++;
							rSample = *iSrc++;
						}
						else
						{
							lSample = *iSrc++;
							rSample = lSample;
						}
						
						lSample = ((lSample * vMul) >> 2);
						rSample = ((rSample * vMul) >> 2);
						
						int midLSample = (lastL + lSample) >> 1;
						int midRSample = (lastR + rSample) >> 1;
						
						lbuf[(i << 1)] = checkClipping(midLSample);
						rbuf[(i << 1)] = checkClipping(midRSample);
						lbuf[(i << 1) + 1] = checkClipping(lSample);
						rbuf[(i << 1) + 1] = checkClipping(rSample);
						
						lastL = lSample;
						lastR = rSample;							
					}
				}	
				break;
				case 4:{
					for(i=0;i<sampleLen;++i)
					{				
						if(soundPlayerCtx->channels == 2)
						{
							lSample = *iSrc++;
							rSample = *iSrc++;
						}
						else
						{
							lSample = *iSrc++;
							rSample = lSample;
						}
						
						lSample = ((lSample * vMul) >> 2);
						rSample = ((rSample * vMul) >> 2);
						
						int midLSample = (lastL + lSample) >> 1;
						int midRSample = (lastR + rSample) >> 1;
						
						int firstLSample = (lastL + midLSample) >> 1;
						int firstRSample = (lastR + midRSample) >> 1;
						
						int secondLSample = (midLSample + lSample) >> 1;
						int secondRSample = (midRSample + rSample) >> 1;
						
						lbuf[(i << 2)] = checkClipping(firstLSample);
						rbuf[(i << 2)] = checkClipping(firstRSample);
						lbuf[(i << 2) + 1] = checkClipping(midLSample);
						rbuf[(i << 2) + 1] = checkClipping(midRSample);
						lbuf[(i << 2) + 2] = checkClipping(secondLSample);
						rbuf[(i << 2) + 2] = checkClipping(secondRSample);
						lbuf[(i << 2) + 3] = checkClipping(lSample);
						rbuf[(i << 2) + 3] = checkClipping(rSample);							
						
						lastL = lSample;
						lastR = rSample;							
					}
				}	
				break;
			}
			VblankUser();
		}
		break;
		case ARM7COMMAND_PSG_COMMAND:
		{				
			struct soundPlayerContext * soundPlayerCtx = soundIPC();
			SCHANNEL_CR(soundPlayerCtx->psgChannel) = soundPlayerCtx->cr;
			SCHANNEL_TIMER(soundPlayerCtx->psgChannel) = soundPlayerCtx->timer;
		}
		break;
		
		#endif
		
		//NDS9: 
		#ifdef ARM9
		
		#endif
	}
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 data0, uint32 data1){
}


#ifdef ARM9
//Callback update sample implementation

__attribute__((section(".itcm")))
void updateSoundContextStreamPlaybackUser(u32 srcFrmt){	//ARM9COMMAND_UPDATE_BUFFER User Req
	updateRequested = true;
			
	// check for formats that can handle not being on an interrupt (better stability)
	// (these formats are generally decoded faster)
	switch(soundData.sourceFmt)
	{
		case SRC_MP3:
			// mono sounds are slower than stereo for some reason
			// so we force them to update faster
			if(soundData.channels != 1)
				return;
			
			break;
		case SRC_FLAC:
		case SRC_STREAM_MP3:
		case SRC_STREAM_AAC:
		case SRC_SID:
			// the rest is handled somewhere else
			return;
	}
	
	// call immediately if the format needs it
	updateStream();
}
#endif

void setupSoundUser(u32 srcFrmtInst){
	#ifdef ARM7
	srcFrmt = srcFrmtInst;
    sndCursor = 0;
	if(multRate != 1 && multRate != 2 && multRate != 4){
		multRate = 1;
	}
	
	mallocData(sampleLen * 2 * multRate);
    
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
	DisableIrq(IRQ_VBLANK);
	
	lastL = 0;
	lastR = 0;
	
	TIMERXDATA(2) = SOUND_FREQ((sndRate * multRate));
	TIMERXCNT(2) = TIMER_DIV_1 | TIMER_ENABLE;
  
	//Timer3 go
	TIMERXDATA(3) = 0x10000 - (sampleLen * 2 * multRate);
	TIMERXCNT(3) = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
	
	REG_IE|=(IRQ_TIMER3);
	#endif
	
	#ifdef ARM9
	#endif
}

void stopSoundUser(u32 srcFrmt){
	#ifdef ARM7
	TIMERXCNT(2) = 0;
	TIMERXCNT(3) = 0;
	
	SCHANNEL_CR(0) = 0;
	SCHANNEL_CR(1) = 0;
	SCHANNEL_CR(2) = 0;
	SCHANNEL_CR(3) = 0;
	
	freeData();
	//irqSet(IRQ_VBLANK, VblankHandler);
	//irqEnable(IRQ_VBLANK);
	EnableIrq(IRQ_VBLANK);
	
	REG_IE&=~(IRQ_TIMER3);
	#endif
	
	#ifdef ARM9
	#endif
}