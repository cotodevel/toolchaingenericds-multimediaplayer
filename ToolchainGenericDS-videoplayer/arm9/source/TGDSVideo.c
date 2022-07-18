#if defined(WIN32) || defined(ARM9)
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

#ifdef WIN32
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "TGDSVideo.h"
#ifdef ARM9
#include "biosTGDS.h"
#include "nds_cp15_misc.h"
#include "dmaTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "soundTGDS.h"
#include "ima_adpcm.h"
#include "main.h"
#include "lz77.h"
#include "posixHandleTGDS.h"
#include "timerTGDS.h"
#endif
#include "lzss9.h"
#if defined (MSDOS) || defined(WIN32)
#include "../ToolchainGenericDSFS/fatfslayerTGDS.h"
#include "../utilities.h"
#include "TGDSTypes.h"
#endif

u8 decompBuf[256*192*2];

//Format name: TVS (ToolchainGenericDS Videoplayer Stream)
//Format Version: 1.3
//Changelog:
//1.3 Add timestamp and synchronize video to audio track.
//1.2 Add LZSS compression
//1.1 add 10 FPS support
//1.0 First version, plays raw uncompressed videoframes
#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct TGDSVideoFrameContext TGDSVideoFrameContextDefinition;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct TGDSVideoFrameContext * TGDSVideoFrameContextReference = NULL;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
bool TGDSVideoPlayback;

#if defined(ARM9) || defined(WIN32)
#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct fd videoHandleFD;

FILE* audioHandleFD = NULL;
#endif

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
volatile u64 DPGAudioStream_SyncSamples=0; //Samples per audio frame

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 DPGAudioStream_PregapSamples=0; //Samples to account always because of the timestamp difference between Video frames

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 vblankCount=1;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 frameInterval=1;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 frameCount=1;


#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
static struct videoFrame videoFrameDef;

//Returns: Total videoFrames found in File handle

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int parseTGDSVideoFile(struct fd * _VideoDecoderFileHandleFD, char * audioFname){
	#ifdef ARM9
	enableFastMode();
	#endif
	char tmpName[MAX_TGDSFILENAME_LENGTH+1];
	char ext[MAX_TGDSFILENAME_LENGTH+1];
	strcpy(tmpName, audioFname);	
	separateExtension(tmpName, ext);
	strlwr(ext);
	TGDSVideoPlayback=false;	
	if(strcmp(ext,".tvs") == 0){
		struct TGDSVideoFrameContext * readTGDSVideoFrameContext = (struct TGDSVideoFrameContext *)TGDSARM9Malloc(sizeof(struct TGDSVideoFrameContext));
		fatfs_seekDirectStructFD(_VideoDecoderFileHandleFD, 0);
		fatfs_readDirectStructFD(_VideoDecoderFileHandleFD, (u8*)readTGDSVideoFrameContext, sizeof(struct TGDSVideoFrameContext) - 4);
		TGDSVideoFrameContextReference = (struct TGDSVideoFrameContext*)&TGDSVideoFrameContextDefinition;
		TGDSVideoFrameContextReference->framesPerSecond = readTGDSVideoFrameContext->framesPerSecond;
		TGDSVideoFrameContextReference->videoFramesTotalCount = readTGDSVideoFrameContext->videoFramesTotalCount;
		TGDSVideoFrameContextReference->videoFrameStartFileOffset = readTGDSVideoFrameContext->videoFrameStartFileOffset;
		TGDSVideoFrameContextReference->videoFrameStartFileSize = readTGDSVideoFrameContext->videoFrameStartFileSize;
		TGDSVideoFrameContextReference->TGDSVideoFileHandle = NULL;		
		
		#ifdef ARM9
		REG_VCOUNT = vblankCount = frameCount = 1;
		#endif
		
		frameInterval = 1;	//(vblankMaxFrame / TGDSVideoFrameContextReference->framesPerSecond); //tick 60/4 times = 15 FPS. //////////2; 
		TGDSARM9Free(readTGDSVideoFrameContext);
		
		return TGDSVideoFrameContextReference->videoFramesTotalCount;
	}
	return -1;
}

//returns: physical struct videoFrame * offset from source FILE handle
#ifdef ARM9
__attribute__((section(".itcm")))
u32 getVideoFrameOffsetFromIndexInFileHandle(int videoFrameIndexFromFileHandle){
	//TGDSVideo methods uses same format for ARM9 and WIN32
	u32 frameOffsetCollectionInFileHandle = ((int)(sizeof(struct TGDSVideoFrameContext) - 4) + (videoFrameIndexFromFileHandle*4));
	UINT nbytes_read;
	u32 offsetInFileRead = 0;
	if( (f_lseek(&videoHandleFD.fil, frameOffsetCollectionInFileHandle) == FR_OK) && (f_read(&videoHandleFD.fil, &offsetInFileRead, sizeof(u32), &nbytes_read) == FR_OK)){
		return offsetInFileRead;
	}
	return -1;
}
#endif

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
int nextVideoFrameOffset = 0;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
int nextVideoFrameFileSize = 0;

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
int TGDSVideoRender(){	
	#ifdef ARM9
	//Any frames loaded? Handle them
	if( (vblankCount != 1) && ((vblankCount % (frameInterval) ) == 0)){
		//render one frame proportional to timestamp sync'd to actual TGDS videoframe
		if(TGDSVideoPlayback == true){
			UINT nbytes_read;
			int frameDescSize = sizeof(struct videoFrame);
			f_lseek(&videoHandleFD.fil, nextVideoFrameOffset);
			f_read(&videoHandleFD.fil, (u8*)decodedBuf, nextVideoFrameFileSize + frameDescSize, &nbytes_read);
			struct videoFrame * frameRendered = (struct videoFrame *)decodedBuf;
			
			if(frameRendered->elapsedTimeStampInMilliseconds < (getTimerCounter()+300)  ){ //0.3s seek time ahead to sync better
				nextVideoFrameOffset = frameRendered->nextVideoFrameOffsetInFile;
				nextVideoFrameFileSize = frameRendered->nextVideoFrameFileSize;
				int decompSize = lzssDecompress((u8*)decodedBuf + frameDescSize, (u8*)decompBufUncached);
				DMA0_SRC = (uint32)(decompBufUncached);
				DMA0_DEST = (uint32)mainBufferDraw;
				DMA0_CR = DMAENABLED | DMAINCR_SRC | DMAINCR_DEST | DMA32BIT | (decompSize>>2);
				
				if(frameCount < TGDSVideoFrameContextReference->videoFramesTotalCount){
					frameCount++;
				}
				else{
					DMA0_CR = 0;
					dmaFillWord(0, 0, (uint32)mainBufferDraw, (uint32)256*192*2);	//clean render buffer
					vblankCount = frameCount = 1;
					nextVideoFrameOffset = TGDSVideoFrameContextReference->videoFrameStartFileOffset;
					nextVideoFrameFileSize = TGDSVideoFrameContextReference->videoFrameStartFileSize;
					TGDSVideoPlayback = false;
					
					//Exit inmediately to TGDS-LM Caller on videoplayback end
					TGDSProjectReturnToCaller(callerNDSBinary);
				}
			}
		}
	}
	#endif
	return 0;
}

//NDS Client implementation End
#endif