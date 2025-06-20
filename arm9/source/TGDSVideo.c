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
#include "loader.h"
#endif
#include "lzss9.h"
#if defined (MSDOS) || defined(WIN32)
#include "../ToolchainGenericDSFS/fatfslayerTGDS.h"
#include "../utilities.h"
#include "TGDSTypes.h"
#include "gui_console_connector.h"
#endif

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
u32 vblankCount=1;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 frameInterval=1;

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
u32 frameCount=1;

u8 decompBuf[256*192*2];
static bool gotFrame = false;

//Returns: Total videoFrames found in File handle

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int parseTGDSVideoFile(struct fd * _VideoDecoderFileHandleFD, char * audioFname){
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
		gotFrame = false;
		TGDSARM9Free(readTGDSVideoFrameContext);
		
		return TGDSVideoFrameContextReference->videoFramesTotalCount;
	}
	return -1;
}

/*
//unused, just for debugging purposes
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
*/

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
	if( (vblankCount != 1) && ((vblankCount % (frameInterval) ) == 0)){
		//Render one videoframe synchronized to its internal timestamp 
		if(TGDSVideoPlayback == true){
			UINT nbytes_read;
			int frameDescSize = sizeof(struct videoFrame);
			#define dlt_time ((int)700*2)
			int curTime = ((int)getTimerCounter()-dlt_time);
			//Read frame once  
			if(gotFrame == false){
				f_lseek(&videoHandleFD.fil, nextVideoFrameOffset);
				f_read(&videoHandleFD.fil, (u8*)decodedBuf, nextVideoFrameFileSize + frameDescSize, &nbytes_read);
				gotFrame = true;
			}

			struct videoFrame * frameRendered = (struct videoFrame *)decodedBuf;
			if( ( (frameRendered->elapsedTimeStampInMilliseconds-dlt_time) < curTime) || (curTime <= 0) ){ //0.7s seek ahead time to sync better
				nextVideoFrameOffset = frameRendered->nextVideoFrameOffsetInFile;
				nextVideoFrameFileSize = frameRendered->nextVideoFrameFileSize;
				int decompSize = lzssDecompress((u8*)decodedBuf + frameDescSize, (u8*)decompBufUncached);
				DMA0_SRC = (uint32)(decompBufUncached);
				DMA0_DEST = (uint32)mainBufferDraw;
				DMA0_CR = DMAENABLED | DMAINCR_SRC | DMAINCR_DEST | DMA32BIT | (decompSize>>2);
				
				if(frameCount < TGDSVideoFrameContextReference->videoFramesTotalCount){
					gotFrame = false;
					frameCount++;
				}
				else{
					DMA0_CR = 0;
					dmaFillWord(0, 0, (uint32)mainBufferDraw, (uint32)256*192*2);	//clean render buffer
					vblankCount = frameCount = 1;
					nextVideoFrameOffset = TGDSVideoFrameContextReference->videoFrameStartFileOffset;
					nextVideoFrameFileSize = TGDSVideoFrameContextReference->videoFrameStartFileSize;
					TGDSVideoPlayback = false;
					
					stopTimerCounter();	//Required, or ARM7 IMA-ADPCM core segfaults due to interrupts working
					ARM7LoadDefaultCore();
					enableFastMode(); //disable Vblank
					enableScreenPowerTimeout();
					
					GUI.GBAMacroMode = false;	//GUI console at bottom screen. 
					TGDSLCDSwap();
					setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
					menuShow();
				}
			}
		}
	}
	#endif
	return 0;
}

//NDS Client implementation End
#endif

void playTVSFile(char * tvsFile){
	//Process TVS file
	int tgdsfd = -1;
	int res = fatfs_open_fileIntoTargetStructFD(tvsFile, "r", &tgdsfd, &videoHandleFD);
	if(parseTGDSVideoFile(&videoHandleFD, tvsFile) > 0){
		disableScreenPowerTimeout();
		ARM7LoadStreamCore();

		GUI.GBAMacroMode = true;	//GUI console at top screen. Bottom screen is playback
		TGDSLCDSwap();
		setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);

		int readFileSize = FS_getFileSizeFromOpenStructFD(&videoHandleFD);
		int predictedClusterCount = (readFileSize / (getDiskClusterSize() * getDiskSectorSize())) + 2;
		nextVideoFrameOffset = TGDSVideoFrameContextReference->videoFrameStartFileOffset;
		nextVideoFrameFileSize = TGDSVideoFrameContextReference->videoFrameStartFileSize;			
		
		//ARM7 ADPCM playback 
		BgMusic(tvsFile);
		
		//wait 2 seconds
		/*
		disableFastMode(); //enable Vblank
		int DSFrame = 0;
		while(DSFrame < 110){
			if(vblankCount == 1){
				DSFrame++;
			}
			IRQVBlankWait();
		}
		*/

		enableFastMode(); //disable Vblank

		if(GUI.GBAMacroMode == false){
			setBacklight(POWMAN_BACKLIGHT_TOP_BIT);
		}
		else{
			setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
		}

		TGDSVideoPlayback = true;
		strcpy(curChosenBrowseFile, tvsFile);
		startTimerCounter(tUnitsMilliseconds, 1, IRQ_TIMER3); //tUnitsMilliseconds equals 1 millisecond/unit. A single unit (1) is the default value for normal timer count-up scenarios. 

		clrscr();
		printf("--");
		printf("--");
		printf(".TVS Playing OK: %s", (char*)tvsFile);
	}
	else{
		TGDSVideoPlayback = false;
		enableFastMode(); //disable Vblank
		enableScreenPowerTimeout();

		GUI.GBAMacroMode = false;	//GUI console at bottom screen. Handle error
		TGDSLCDSwap();
		setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
		
		clrscr();
		printf("--");
		printf("--");
		printf("Not a .TVS File: %s", (char*)tvsFile);
		printf("Press (B) to exit.");
		
		while(1==1){
			scanKeys();
			if (keysDown() & KEY_B){	
				break;
			}
			IRQVBlankWait();
		}
		menuShow();
	}
}

u8 savedDefaultCore[96*1024];

void ARM7LoadStreamCore(){
	//Playback:
	//Let decoder close context so we can start again
	//Audio stop here
	closeSound();
	
	updateStream();	
	updateStream();
	updateStream();
	updateStream();
	
	updateStream();	
	updateStream();
	updateStream();
	updateStream();
	
	updateStream();	
	updateStream();
	updateStream();
	updateStream();
	
	updateStream();	
	updateStream();
	updateStream();
	updateStream();
	haltARM7(); //Required, or ARM7 IMA-ADPCM core segfaults due to interrupts working
	
	bool isTGDSCustomConsole = true;	//set default console or custom console: custom console
	GUI_init(isTGDSCustomConsole);
	GUI_clear();

	WRAM_CR = WRAM_0KARM9_32KARM7;
	
	//Reload VRAM Core here
	REG_IME = 0;
	u32 * payload = getTGDSMBV3ARM7AudioCore();
	executeARM7Payload((u32)0x02380000, 96*1024, payload);
	BgMusicOff();
	REG_IME = 1;
	haltARM7(); //Required, or ARM7 IMA-ADPCM core segfaults due to interrupts working
}

void ARM7LoadDefaultCore(){
	BgMusicOff();
	haltARM7(); //Required, or ARM7 IMA-ADPCM core segfaults due to interrupts working
	
	bool project_specific_console = false;	//set default console or custom console: custom console
	GUI_init(project_specific_console);
	GUI_clear();
	
	//Stop playback. Go back to IWRAM Core
	REG_IME = 0;
	executeARM7Payload((u32)0x02380000, 96*1024, (u32*)savedDefaultCore);
	WRAM_CR = WRAM_32KARM9_0KARM7;
	REG_IME = 1;
}