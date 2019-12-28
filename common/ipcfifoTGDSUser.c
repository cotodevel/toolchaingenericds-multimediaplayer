
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

#ifdef ARM7
#include <string.h>

#include "main.h"
#include "wifi_arm7.h"
#include "spifwTGDS.h"
#include "click_raw.h"

static inline s16 checkClipping(int data)
{
	if(data > 32767)
		return 32767;
	if(data < -32768)
		return -32768;
	
	return data;
}

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
void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2){
	
	switch (cmd1) {
		//NDS7: 
		#ifdef ARM7
		case ARM7COMMAND_START_SOUND:{
			SetupSound();
		}
		break;
		case ARM7COMMAND_STOP_SOUND:{
			StopSound();
		}
		break;
		case ARM7COMMAND_SOUND_SETRATE:{
			sndRate = cmd2;
		}
		break;
		case ARM7COMMAND_SOUND_SETLEN:{
			sampleLen = cmd2;
		}
		break;
		case ARM7COMMAND_SOUND_SETMULT:{
			multRate = cmd2;
		}
		break;
		case ARM7COMMAND_SOUND_COPY:
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
			
			u32 i;
			int vMul = soundIPC->volume;
			int lSample = 0;
			int rSample = 0;
			s16 *arm9LBuf = soundIPC->arm9L;
			s16 *arm9RBuf = soundIPC->arm9R;
			
			switch(multRate)
			{
				case 1:{
					for(i=0;i<sampleLen;++i)
					{
						lSample = ((*arm9LBuf++) * vMul) >> 2;
						rSample = ((*arm9RBuf++) * vMul) >> 2;
						
						*lbuf++ = checkClipping(lSample);
						*rbuf++ = checkClipping(rSample);
					}
				}	
				break;
				case 2:{
					for(i=0;i<sampleLen;++i)
					{
						lSample = ((*arm9LBuf++) * vMul) >> 2;
						rSample = ((*arm9RBuf++) * vMul) >> 2;
						
						int midLSample = (lastL + lSample) >> 1;
						int midRSample = (lastR + rSample) >> 1;
						
						*lbuf++ = checkClipping(midLSample);
						*rbuf++ = checkClipping(midRSample);
						*lbuf++ = checkClipping(lSample);
						*rbuf++ = checkClipping(rSample);
						
						lastL = lSample;
						lastR = rSample;
					}
				}	
				break;
				case 4:{
					// unrolling this one out completely because it's soo much slower
					
					for(i=0;i<sampleLen;++i)
					{
						lSample = ((*arm9LBuf++) * vMul) >> 2;
						rSample = ((*arm9RBuf++) * vMul) >> 2;
						
						int midLSample = (lastL + lSample) >> 1;
						int midRSample = (lastR + rSample) >> 1;
						
						int firstLSample = (lastL + midLSample) >> 1;
						int firstRSample = (lastR + midRSample) >> 1;
						
						int secondLSample = (midLSample + lSample) >> 1;
						int secondRSample = (midRSample + rSample) >> 1;
						
						*lbuf++ = checkClipping(firstLSample);
						*rbuf++ = checkClipping(firstRSample);
						*lbuf++ = checkClipping(midLSample);
						*rbuf++ = checkClipping(midRSample);
						*lbuf++ = checkClipping(secondLSample);
						*rbuf++ = checkClipping(secondRSample);
						*lbuf++ = checkClipping(lSample);
						*rbuf++ = checkClipping(rSample);							
						
						lastL = lSample;
						lastR = rSample;							
					}
				}	
				break;
			}
			VblankUser();
		}
		break;
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
			
			s16 *iSrc = soundIPC->interlaced;
			u32 i = 0;
			int vMul = soundIPC->volume;
			int lSample = 0;
			int rSample = 0;
			
			switch(multRate)
			{
				case 1:{
					if(soundIPC->channels == 2)
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
						if(soundIPC->channels == 2)
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
						if(soundIPC->channels == 2)
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
			SCHANNEL_CR(soundIPC->psgChannel) = soundIPC->cr;
			SCHANNEL_TIMER(soundIPC->psgChannel) = soundIPC->timer;
		}
		break;
		#endif
		
		//NDS9: 
		#ifdef ARM9
		case ARM9COMMAND_UPDATE_BUFFER:{
			
			
			REG_IME = 0;
			{
				updateRequested = true;
				
				// check for formats that can handle not being on an interrupt (better stability)
				// (these formats are generally decoded faster)
				switch(soundData.sourceFmt)
				{
					/*
					case SRC_MP3:
						// mono sounds are slower than stereo for some reason
						// so we force them to update faster
						if(soundData.channels != 1)
							return;
						
						break;
					*/
					case SRC_WAV:
					case SRC_FLAC:
					//case SRC_STREAM_MP3:
					case SRC_STREAM_AAC:
					case SRC_SID:
						// these will be played next time it hits in the main screen
						// theres like 4938598345 of the updatestream checks in the 
						// main code
						return;
				}
				
				// call immediately if the format needs it
				updateStreamLoop();
			}
			REG_IME = 1;
		}	
		break;
		case ARM9COMMAND_SAVE_DATA:{
			//copyChunk();
		}
		break;
		#endif
	}
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2){
}

void setSoundRegion(TransferSound * sndRegion){
	struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;	
	TGDSUSERIPC->soundData = sndRegion;
}

TransferSound * getSoundRegion(){
	struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;
	return (TransferSound *)TGDSUSERIPC->soundData;
}