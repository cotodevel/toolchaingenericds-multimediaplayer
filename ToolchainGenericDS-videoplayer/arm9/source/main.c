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
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dswnifi_lib.h"
#include "keypadTGDS.h"
#include "fileBrowse.h"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.
#include "biosTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "dldi.h"
#include "global_settings.h"
#include "posixHandleTGDS.h"
#include "TGDSMemoryAllocator.h"
#include "consoleTGDS.h"
#include "soundTGDS.h"
#include "dmaTGDS.h"
#include "nds_cp15_misc.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "ima_adpcm.h"
#include "libndsFIFO.h"
#include "xenofunzip.h"
#include "cartHeader.h"
#include "InterruptsARMCores_h.h"
#include "typedefsTGDS.h"
#include "videoTGDS.h"
#include "math.h"
#include "imagepcx.h"
#include "loader.h"
#include "dswnifi_lib.h"
#include "gui_console_connector.h"
#include <stdio.h>
#include "TGDSVideo.h"
#include "debugNocash.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lz77.h"
#include "interrupts.h"
#include "timerTGDS.h"

//TGDS-MB ARM7 Bootldr
#include "arm7bootldr.h"
#include "arm7bootldr_twl.h"

//ARM7 VRAM core
#include "arm7vram.h"
#include "arm7vram_twl.h"

u32 * getTGDSMBV3ARM7Bootloader(){	//Required by ToolchainGenericDS-multiboot v3
	if(__dsimode == false){
		return (u32*)&arm7bootldr[0];	
	}
	else{
		return (u32*)&arm7bootldr_twl[0];
	}
}

u32 * getTGDSARM7VRAMCore(){	//TGDS Project specific ARM7 VRAM Core
	if(__dsimode == false){
		return (u32*)&arm7vram[0];	
	}
	else{
		return (u32*)&arm7vram_twl[0];
	}
}

struct FileClassList * menuIteratorfileClassListCtx = NULL;
char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH];
char globalPath[MAX_TGDSFILENAME_LENGTH];
static int curFileIndex = 0;
static bool pendingPlay = false;

//TGDS Soundstreaming API
int internalCodecType = SRC_NONE; //Returns current sound stream format: WAV, ADPCM or NONE
struct fd * _FileHandleVideo = NULL; 
struct fd * _FileHandleAudio = NULL;

bool stopSoundStreamUser(){
	return stopSoundStream(_FileHandleVideo, _FileHandleAudio, &internalCodecType);
}

void closeSoundUser(){
	//Stubbed. Gets called when closing an audiostream of a custom audio decoder
}

void menuShow(){
	if(getTGDSDebuggingState() == true){
		clrscr();
		printf("     ");
		printf("     ");
		printf("Current file: %s ", curChosenBrowseFile);
		printf("ToolchainGenericDS-videoplayer ");
		printf("(Select): This menu. ");
		printf("(Start) then (A): Play TVS video sequence. ");
		printf("Available program memory: %d >%d", TGDSMallocFreeMemory9(), TGDSPrintfColor_Cyan);
		printf("Note: Press Start again to show the menu. >%d", TGDSPrintfColor_Yellow);
		printf("Available heap memory: %d >%d", getMaxRam(), TGDSPrintfColor_Cyan);
	}
}


static char thisTGDSProject[MAX_TGDSFILENAME_LENGTH];
static char thisArgv2[3][MAX_TGDSFILENAME_LENGTH];

char fnameRead[256];

__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TGDSProjectReturnToCaller(char * NDSPayload){	//TGDS-Linked Module implementation
	//stop streaming
	BgMusicOff();
	
	swiDelay(10000); //wait a little
	
	REG_IME = 0;
	MPUSet();
	REG_IME = 1;
	
	memset(fnameRead, 0, sizeof(fnameRead));
	strcpy(fnameRead, "0:/");
	strcat(fnameRead, NDSPayload);
	
	if(__dsimode == true){
		strcat(fnameRead, ".srl");
	}
	else{
		strcat(fnameRead, ".nds");
	}
	
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);
	
	clrscr();
	printf("--");
	printf("--");
	printf("--");
	printf("trying to go back:");
	printf("%s", fnameRead);
	
	strcpy(thisTGDSProject, "0:/");
	strcat(thisTGDSProject, "ToolchainGenericDS-multiboot");
	if(__dsimode == true){
		strcat(thisTGDSProject, ".srl");
	}
	else{
		strcat(thisTGDSProject, ".nds");
	}
	
	//October 14 2024: TGDS-mb v3 chainload example working. If you call TGDS-videoplayer from TGDS-multiboot v3, it'll chainload into "fnameRead" from Arg1, and pass "0:/stub.bin" from Arg2
	/*
	memset(thisArgv2, 0, sizeof(thisArgv2));
	strcpy(&thisArgv2[0][0], TGDSPROJECTNAME);	//Arg0:	This Binary loaded
	strcpy(&thisArgv2[1][0], fnameRead);	//Arg1:	NDS Binary reloaded
	strcpy(&thisArgv2[2][0], (char*)"0:/stub.bin");	//Arg2: NDS Binary ARG0
	u32 * payload = getTGDSARM7VRAMCore();
	if(TGDSMultibootRunNDSPayload(fnameRead, (u8*)payload, 3, (char*)&thisArgv2) == false){  //Should fail it returns false. 
		printf("boot failed");
	}
	*/

	memset(thisArgv2, 0, sizeof(thisArgv2));
	strcpy(&thisArgv2[0][0], TGDSPROJECTNAME);	//Arg0:	This Binary loaded
	strcpy(&thisArgv2[1][0], fnameRead);	//Arg1:	NDS Binary reloaded
	u32 * payload = getTGDSARM7VRAMCore();
	if(TGDSMultibootRunNDSPayload(fnameRead, (u8*)payload, 2, (char*)&thisArgv2) == false){  //Should fail it returns false. 
		printf("boot failed");
	}
	
	while(1==1){
		swiDelay(1);
	}
}

char callerNDSBinary[256];
char args[8][MAX_TGDSFILENAME_LENGTH];
char *argvs[8];

void playTVSFile(char * tvsFile){
	//Process TVS file
	int tgdsfd = -1;
	int res = fatfs_open_fileIntoTargetStructFD(tvsFile, "r", &tgdsfd, &videoHandleFD);
	if(parseTGDSVideoFile(&videoHandleFD, tvsFile) > 0){
		int readFileSize = FS_getFileSizeFromOpenStructFD(&videoHandleFD);
		int predictedClusterCount = (readFileSize / (getDiskClusterSize() * getDiskSectorSize())) + 2;
		nextVideoFrameOffset = TGDSVideoFrameContextReference->videoFrameStartFileOffset;
		nextVideoFrameFileSize = TGDSVideoFrameContextReference->videoFrameStartFileSize;			
		
		//ARM7 ADPCM playback 
		BgMusic(tvsFile);
		
		if(GUI.GBAMacroMode == false){
			setBacklight(POWMAN_BACKLIGHT_TOP_BIT);
		}
		else{
			setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
		}

		TGDSVideoPlayback = true;
		strcpy(curChosenBrowseFile, tvsFile);
		startTimerCounter(tUnitsMilliseconds, 1); //tUnitsMilliseconds equals 1 millisecond/unit. A single unit (1) is the default value for normal timer count-up scenarios. 

		clrscr();
		printf("--");
		printf("--");
		printf(".TVS Playing OK: %s", (char*)tvsFile);
	}
	else{
		TGDSVideoPlayback = false;

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
		
		TGDSProjectReturnToCaller(callerNDSBinary);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int argc, char **argv) {
	/*			TGDS 1.6 Standard ARM9 Init code start	*/
	//Save Stage 1: IWRAM ARM7 payload: NTR/TWL (0x03800000)
	memcpy((void *)TGDS_MB_V3_ARM7_STAGE1_ADDR, (const void *)0x02380000, (int)(96*1024));	//
	coherent_user_range_by_size((uint32)TGDS_MB_V3_ARM7_STAGE1_ADDR, (int)(96*1024)); //		also for TWL binaries 
	
	//Execute Stage 2: VRAM ARM7 payload: NTR/TWL (0x06000000)
	u32 * payload = getTGDSARM7VRAMCore();
	executeARM7Payload((u32)0x02380000, 96*1024, payload);
	
	bool isTGDSCustomConsole = true;	//set default console or custom console: custom console
	GUI_init(isTGDSCustomConsole);
	GUI_clear();

	bool isCustomTGDSMalloc = true;
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup(isCustomTGDSMalloc));
	sint32 fwlanguage = (sint32)getLanguage();

	int ret=FS_init();
	if (ret != 0){
		printf("%s: FS Init error: %d >%d", TGDSPROJECTNAME, ret, TGDSPrintfColor_Red);
		while(1==1){
			swiDelay(1);
		}
	}
	
	asm("mcr	p15, 0, r0, c7, c10, 4");
	flush_icache_all();
	flush_dcache_all();
	/*			TGDS 1.6 Standard ARM9 Init code end	*/
	
	
	REG_IME = 0;
	setSnemulDSSpecial0xFFFF0000MPUSettings();
	//TGDS-Projects -> legacy NTR TSC compatibility
	if(__dsimode == true){
		TWLSetTouchscreenTWLMode();
	}
	REG_IME = 1;
	
	setupDisabledExceptionHandler();
	
	strcpy(globalPath, "/");
	menuShow();
	
	if(getTGDSDebuggingState() == true){
		if (0 != argc ) {
			int i;
			for (i=0; i<argc; i++) {
				if (argv[i]) {
					printf("[%d] %s ", i, argv[i]);
				}
			}
		} 
		else {
			printf("No arguments passed!");
		}
	}
	
	//Discard FIFO errors
	if(REG_IPC_FIFO_CR & IPC_FIFO_ERROR){ 
		REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	}

	//Play TVS by argv
	if((argv != NULL) && (argv[0] != NULL)){
		strcpy(callerNDSBinary, (char *)argv[0]);
	}
	if(argc > 2){
		playTVSFile((char *)argv[2]);
	}
	else{
		TGDSVideoPlayback = false;
		clrscr();
		printf("--");
		printf("--");
		printf("TGDS-videoplayer requires a TVS file chosen >%d", TGDSPrintfColor_Red);
		printf("through ARGV. >%d", TGDSPrintfColor_Red);
		printf("Press (B) to exit.");
		
		while(1==1){
			scanKeys();
			if (keysDown() & KEY_B){	
				break;
			}
			IRQVBlankWait();
		}
		
		TGDSProjectReturnToCaller(callerNDSBinary);
	}

	while(1) {
		scanKeys();	
		if (keysDown() & KEY_UP){
			volumeUp(0, 0);
			menuShow();
			scanKeys();
			while(keysHeld() & KEY_UP){
				scanKeys();
				IRQWait(0, IRQ_VBLANK);
			}
		}
		
		if (keysDown() & KEY_DOWN){
			volumeDown(0, 0);
			menuShow();
			scanKeys();
			while(keysHeld() & KEY_DOWN){
				scanKeys();
				IRQWait(0, IRQ_VBLANK);
			}
		}
		
		TGDSVideoRender();
	}

	return 0;
}


void updateStreamCustomDecoder(u32 srcFrmt){

}

void freeSoundCustomDecoder(u32 srcFrmt){

}