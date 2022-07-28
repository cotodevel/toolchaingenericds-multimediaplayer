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
#include "click_raw.h"
#include "ima_adpcm.h"
#include "libndsFIFO.h"
#include "xenofunzip.h"
#include "cartHeader.h"
#include "InterruptsARMCores_h.h"
#include "typedefsTGDS.h"
#include "VideoGL.h"
#include "videoTGDS.h"
#include "math.h"
#include "imagepcx.h"
#include "Texture_Cube.h"
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

struct FileClassList * menuIteratorfileClassListCtx = NULL;
char curChosenBrowseFile[256+1];
char globalPath[MAX_TGDSFILENAME_LENGTH+1];
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

__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Os")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TGDSProjectReturnToCaller(char * NDSPayload){	//TGDS-Linked Module implementation
	REG_IME = 0;
	MPUSet();
	REG_IME = 1;

	char fnameRead[256];
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
	char thisArgv[4][MAX_TGDSFILENAME_LENGTH];
	memset(thisArgv, 0, sizeof(thisArgv));
	
	clrscr();
	printf("--");
	printf("--");
	printf("--");
	printf("trying to go back:");
	printf("%s", fnameRead);
	//while(1==1){}
	
	char thisTGDSProject[MAX_TGDSFILENAME_LENGTH+1];
	strcpy(thisTGDSProject, "0:/");
	strcat(thisTGDSProject, "ToolchainGenericDS-multiboot");
	if(__dsimode == true){
		strcat(thisTGDSProject, ".srl");
	}
	else{
		strcat(thisTGDSProject, ".nds");
	}
	
	strcpy(&thisArgv[0][0], thisTGDSProject);	//Arg0:	This Binary loaded
	strcpy(&thisArgv[1][0], fnameRead);	//Arg1:	Chainload caller: TGDS-MB
	strcpy(&thisArgv[2][0], thisTGDSProject);	//Arg2:	NDS Binary reloaded through ChainLoad
	strcpy(&thisArgv[3][0], (char*)"0:/stub.bin");//Arg3: NDS Binary reloaded through ChainLoad's ARG0
	addARGV(4, (char*)&thisArgv);		
	
	
	if(TGDSMultibootRunNDSPayload(thisTGDSProject) == false){  //Should fail it returns false. 
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
		char * filen = FS_getFileName(tvsFile);
		strcat(filen, ".ima");
		u32 returnStatus = setupDirectVideoFrameRender(&videoHandleFD, (char*)&filen[2]);
		
		setBacklight(POWMAN_BACKLIGHT_TOP_BIT);				
		TGDSVideoPlayback = true;
		strcpy(curChosenBrowseFile, tvsFile);
		startTimerCounter(tUnitsMilliseconds); //timer go
	}
	else{
		TGDSVideoPlayback = false;
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

__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int argc, char **argv) {
	/*			TGDS 1.6 Standard ARM9 Init code start	*/
	bool isTGDSCustomConsole = true;	//set default console or custom console: default console
	GUI_init(isTGDSCustomConsole);
	GUI_clear();
	
	//xmalloc init removes args, so save them
	int i = 0;
	for(i = 0; i < argc; i++){
		argvs[i] = argv[i];
	}

	bool isCustomTGDSMalloc = true;
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, isCustomTGDSMalloc, TGDSDLDI_ARM7_ADDRESS));
	sint32 fwlanguage = (sint32)getLanguage();
	
	//argv destroyed here because of xmalloc init, thus restore them
	for(i = 0; i < argc; i++){
		argv[i] = argvs[i];
	}

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
	
	/////////////////////////////////////////////////////////Reload TGDS Proj///////////////////////////////////////////////////////////
	char tmpName[256];
	char ext[256];	
	char TGDSProj[256];
	char curChosenBrowseFile[256];
	strcpy(TGDSProj,"0:/");
	strcat(TGDSProj, "ToolchainGenericDS-multiboot");
	if(__dsimode == true){
		strcat(TGDSProj, ".srl");
	}
	else{
		strcat(TGDSProj, ".nds");
	}
	//Force ARM7 reload once 
	if( 
		(argc < 3) 
		&& 
		(strncmp(argv[1], TGDSProj, strlen(TGDSProj)) != 0) 	
	){
		REG_IME = 0;
		MPUSet();
		REG_IME = 1;
		char startPath[MAX_TGDSFILENAME_LENGTH+1];
		strcpy(startPath,"/");
		strcpy(curChosenBrowseFile, TGDSProj);
		
		char thisTGDSProject[MAX_TGDSFILENAME_LENGTH+1];
		strcpy(thisTGDSProject, "0:/");
		strcat(thisTGDSProject, TGDSPROJECTNAME);
		if(__dsimode == true){
			strcat(thisTGDSProject, ".srl");
		}
		else{
			strcat(thisTGDSProject, ".nds");
		}
		
		//Boot .NDS file! (homebrew only)
		strcpy(tmpName, curChosenBrowseFile);
		separateExtension(tmpName, ext);
		strlwr(ext);
		
		//pass incoming launcher's ARGV0
		char arg0[256];
		int newArgc = 3;
		if (argc > 2) {
			printf(" ---- test");
			printf(" ---- test");
			printf(" ---- test");
			printf(" ---- test");
			printf(" ---- test");
			printf(" ---- test");
			printf(" ---- test");
			printf(" ---- test");
			
			//arg 0: original NDS caller
			//arg 1: this NDS binary
			//arg 2: this NDS binary's ARG0: filepath
			strcpy(arg0, (const char *)argv[2]);
			newArgc++;
		}
		//or else stub out an incoming arg0 for relaunched TGDS binary
		else {
			strcpy(arg0, (const char *)"0:/incomingCommand.bin");
			newArgc++;
		}
		//debug end
		
		char thisArgv[4][MAX_TGDSFILENAME_LENGTH];
		memset(thisArgv, 0, sizeof(thisArgv));
		strcpy(&thisArgv[0][0], thisTGDSProject);	//Arg0:	This Binary loaded
		strcpy(&thisArgv[1][0], curChosenBrowseFile);	//Arg1:	Chainload caller: TGDS-MB
		strcpy(&thisArgv[2][0], thisTGDSProject);	//Arg2:	NDS Binary reloaded through ChainLoad
		strcpy(&thisArgv[3][0], (char*)&arg0[0]);//Arg3: NDS Binary reloaded through ChainLoad's ARG0
		addARGV(newArgc, (char*)&thisArgv);				
		if(TGDSMultibootRunNDSPayload(curChosenBrowseFile) == false){ //should never reach here, nor even return true. Should fail it returns false
			
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	REG_IME = 0;
	setSnemulDSSpecial0xFFFF0000MPUSettings();
	REG_IME = 1;
	
	strcpy(globalPath, "/");
	menuShow();
	
	//ARGV Implementation test
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