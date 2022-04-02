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

//TGDS-target project ARM7 NTR/TWL payloads
#include "arm7_ntr.h" 

////////////////These payloads are prebuilt from ToolchainGenericDS-multiboot project. 
#include "arm7bootldr.h"
#include "arm7bootldr_twl.h"

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

static inline void menuShow(){
	struct TGDS_Linked_Module * TGDSLinkedModuleCtx = (struct TGDS_Linked_Module *)((int)0x02300000 - (0x8000  + 0x1000));
	clrscr();
	printf("     ");
	printf("     ");
	printf("Caller TGDS-Project: %s", (char*)&TGDSLinkedModuleCtx->TGDSMainAppName);
	printf("Current file: %s ", curChosenBrowseFile);
	printf("ToolchainGenericDS-videoplayer ");
	printf("(Select): This menu. ");
	printf("(Start) then (A): Play TVS video sequence. ");
	printf("Available program memory: %d >%d", TGDSMallocFreeMemory9(), TGDSPrintfColor_Cyan);
	printf("Note: Press Start again to show the menu. >%d", TGDSPrintfColor_Yellow);
	printf("Available heap memory: %d >%d", getMaxRam(), TGDSPrintfColor_Cyan);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void TGDSProjectReturnFromLinkedModuleDeciderStub() {	//TGDS-Linked Module implementation
	struct TGDS_Linked_Module * TGDSLinkedModuleCtx = (struct TGDS_Linked_Module *)((int)0x02300000 - (0x8000  + 0x1000));
	char thisArgv[3][MAX_TGDSFILENAME_LENGTH];
	memset(thisArgv, 0, sizeof(thisArgv));
	strcpy(&thisArgv[0][0], TGDSPROJECTNAME);	//Arg0:	NDS Binary loaded
	strcpy(&thisArgv[1][0], (char*)&TGDSLinkedModuleCtx->TGDSMainAppName);					//Arg1: ARGV0
	addARGV(2, (char*)&thisArgv);
	
	char fnameRead[256];
	memset(fnameRead, 0, sizeof(fnameRead));
	strcpy(fnameRead, "0:/");
	strcat(fnameRead, TGDSLinkedModuleCtx->TGDSMainAppName);
	
	if(__dsimode == true){
		strcat(fnameRead, ".srl");
	}
	else{
		strcat(fnameRead, ".nds");
	}
	
	clrscr();
	printf(" ---- ");
	printf(" ---- ");
	printf(" ---- ");
	printf(" ---- ");
	
	printf("trying: %s", fnameRead);
	
	memset(thisArgv, 0, sizeof(thisArgv));
	strcpy(&thisArgv[0][0], TGDSPROJECTNAME);	//Arg0:	This Binary loaded
	strcpy(&thisArgv[1][0], fnameRead);	//Arg1:	NDS Binary reloaded
	strcpy(&thisArgv[2][0], "");					//Arg2: NDS Binary ARG0
	addARGV(3, (char*)&thisArgv);
	if(TGDSMultibootRunNDSPayload(fnameRead) == false){  //Should fail it returns false. (Audio track)
		printf("boot failed");
	}
	while(1==1){
		swiDelay(1);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void reloadARM7PlayerPayload(u32 arm7entryaddress, int arm7BootCodeSize){
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
	
	//NTR ARM7 payload
	if(__dsimode == false){
		coherent_user_range_by_size((u32)&arm7bootldr[0], arm7BootCodeSize);
		setValueSafe(&fifomsg[0], (u32)&arm7bootldr[0]);
	}
	//TWL ARM7 payload
	else{
		coherent_user_range_by_size((u32)&arm7bootldr_twl[0], arm7BootCodeSize);
		setValueSafe(&fifomsg[0], (u32)&arm7bootldr_twl[0]);
	}
	setValueSafe(&fifomsg[1], (u32)arm7BootCodeSize);
	setValueSafe(&fifomsg[2], (u32)arm7entryaddress);
	SendFIFOWords(FIFO_ARM7_RELOAD, 0xFF);
}

//ToolchainGenericDS-LinkedModule User implementation: Stubbed here as TGDS-LM isn't Parent TGDS App
char args[8][MAX_TGDSFILENAME_LENGTH];
char *argvs[8];

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int TGDSProjectReturnFromLinkedModule(){	//Stubbed TGDS-LinkedModule implementation, since it can't call itself when returning.
	return -1;
}

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
		
		//leave TGDS-LM payload
		leaveTGDSLMNow();
	}
}

//User call to leave TGDS-LM properly
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void leaveTGDSLMNow(){
	SendFIFOWords(FIFO_TGDSVIDEOPLAYER_STOPSOUND, 0xFF);
	swiDelay(900);
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);
	swiDelay(900);
	REG_IME = 0;
	REG_IE = 0;
	leaveTGDSLM((u32)&TGDSProjectReturnFromLinkedModuleDeciderStub);	
}

__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int argc, char **argv) {
	REG_IME = 0;
	setSnemulDSSpecial0xFFFF0000MPUSettings();
	REG_IME = 1;
	
	//Generate TGDS-LM context
	struct TGDS_Linked_Module * TGDSLinkedModuleCtx = (struct TGDS_Linked_Module *)((int)0x02300000 - (0x8000  + 0x1000));
	TGDSLinkedModuleCtx->returnAddressTGDSLinkedModule = (u32)&TGDSProjectReturnFromLinkedModuleDeciderStub;	//Implemented when TGDS-LM boots
	
	/*			TGDS 1.6 Standard ARM9 Init code start	*/
	bool isTGDSCustomConsole = true;	//set default console or custom console: custom console
	GUI_init(isTGDSCustomConsole);
	GUI_clear();
	
	//xmalloc init removes args, so save them
	int i = 0;
	for(i = 0; i < argc; i++){
		argvs[i] = argv[i];
	}
	
	//ARM7 reload
	{
		u8 * TGDS_LM_ARM7LZSSPayloadSourceBuffer = (u8*)&TGDSLinkedModuleCtx->TGDS_LM_ARM7PAYLOADLZSS[0];
		int TGDS_LM_ARM7LZSSPayloadSize = TGDSLinkedModuleCtx->TGDS_LM_ARM7PAYLOADLZSSSize;
		
		//Use newly built custom ARM7 payload. Note: These values are hardcoded from ARM7 NTR/TWL linker settings
		{
			if(__dsimode == false){
				TGDS_LM_ARM7LZSSPayloadSize = TGDSLinkedModuleCtx->TGDS_LM_ARM7PAYLOADLZSSSize = arm7_ntr_size;
				memcpy ((void *)TGDS_LM_ARM7LZSSPayloadSourceBuffer, (const void *)&arm7_ntr[0], TGDS_LM_ARM7LZSSPayloadSize);
				coherent_user_range_by_size((u32)TGDS_LM_ARM7LZSSPayloadSourceBuffer, TGDS_LM_ARM7LZSSPayloadSize);
			}
			//TWL ARM7 payload
			else{
				//todo
			}
			TGDSLinkedModuleCtx->arm7EntryAddress = (u32)0x03800000;
			TGDSLinkedModuleCtx->arm7BootCodeSize = (int)(*(unsigned int *)TGDS_LM_ARM7LZSSPayloadSourceBuffer >> 8);
		}
		
		if(TGDS_LM_ARM7LZSSPayloadSize > 0){
			//Stage 1:
			//Reload internal bootstub ARM7 payload
			reloadStatus = (u32)0xFFFFFFFF;
			reloadARM7PlayerPayload((u32)0x023D0000, 64*1024);
			while(reloadStatus == (u32)0xFFFFFFFF){
				swiDelay(1);	
			}
			
			//Stage 2:
			//Reload target TGDS-LM ARM7 payload here
			memset((void *)ARM7_PAYLOAD, 0x0, 64*1024);
			coherent_user_range_by_size((uint32)ARM7_PAYLOAD, 64*1024);
			
			//LZSS Decompress
			swiDecompressLZSSWram((void *)TGDS_LM_ARM7LZSSPayloadSourceBuffer,(void *)ARM7_PAYLOAD);
			coherent_user_range_by_size((uint32)ARM7_PAYLOAD, 64*1024);
			
			//Boot			
			struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
			uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueueSharedRegion[0];
			setValueSafe(&fifomsg[0], (u32)TGDSLinkedModuleCtx->arm7EntryAddress);
			setValueSafe(&fifomsg[1], (u32)TGDSLinkedModuleCtx->arm7BootCodeSize);
			SendFIFOWords(FIFO_TGDSMBRELOAD_SETUP, 0xFF);
			while (getValueSafe(&fifomsg[0]) == (u32)TGDSLinkedModuleCtx->arm7EntryAddress){
				swiDelay(1);
			}
		}
	}
	
	/*			TGDS 1.6 Standard ARM9 Init code start	*/
	isTGDSCustomConsole = true;	//set default console or custom console: custom console
	GUI_init(isTGDSCustomConsole);
	GUI_clear();
	
	bool isCustomTGDSMalloc = true;
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, isCustomTGDSMalloc, TGDSDLDI_ARM7_ADDRESS));
	sint32 fwlanguage = (sint32)getLanguage();
	
	//argv destroyed here because of xmalloc init, thus restore them
	for(i = 0; i < argc; i++){
		argv[i] = argvs[i];
	}

	int ret=FS_init();
	if (ret == 0)
	{
		printf("FS Init ok.");
	}
	else{
		printf("FS Init error: %d :(", ret);
		while(1==1){
			*(u32*)0x08000000 = (u32)0xc070FFFF;
		}
	}
	
	//Reload TGDSLMARM7 flags here because hot-reloaded payload
	struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
	coherent_user_range_by_size((uint32)TGDSIPC, sizeof(struct sIPCSharedTGDS));
	TGDSLinkedModuleCtx->TGDSLMARM7Flags = TGDSIPC->TGDSLMARM7Flags;
	//TGDSLinkedModuleCtx->TGDSLMARM9Flags = TGDSIPC->TGDSLMARM9Flags; //TGDS-LM ARM9 are generated the moment this payload booted up. Must ignore this or ARM9 flags are destroyed by ARM7 payload reload (IPC)
	
	//Enable wifi only if ARM9 payload and current reloaded ARM7 payload has the features available
	if(
		(wifiswitchDsWnifiModeARM9LibUtilsCallback != NULL)
		&&
		((TGDSLinkedModuleCtx->TGDSLMARM7Flags & TGDS_LM_WIFI_ABILITY) == TGDS_LM_WIFI_ABILITY)
		&&
		((TGDSLinkedModuleCtx->TGDSLMARM9Flags & TGDS_LM_WIFI_ABILITY) == TGDS_LM_WIFI_ABILITY)
	){
		wifiswitchDsWnifiModeARM9LibUtilsCallback(dswifi_idlemode);
	}
	
	asm("mcr	p15, 0, r0, c7, c10, 4");
	flush_icache_all();
	flush_dcache_all();
	/*			TGDS 1.6 Standard ARM9 Init code end	*/
	
	strcpy(globalPath, "/");
	menuShow();
	
	//ARGV Implementation test
	if (0 != argc ) {
		int i;
		for (i=0; i<argc; i++) {
			if (argv[i]) {
				printf("[%d] %s \n", i, argv[i]);
			}
		}
	} 
	else {
		printf("No arguments passed!\n");
	}
	
	//Discard FIFO errors
	if(REG_IPC_FIFO_CR & IPC_FIFO_ERROR){ 
		REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	}
	
	//Play TVS by argv
	if(argc > 2){
		playTVSFile((char *)argv[2]);
	}
	else{
		TGDSVideoPlayback = false;
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
		
		//leave TGDS-LM payload
		leaveTGDSLMNow();
	}
	
	while(1) {
		scanKeys();	
		/*
		if (keysDown() & KEY_START){
			SendFIFOWords(FIFO_TGDSVIDEOPLAYER_STOPSOUND, 0xFF);
			setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);
			while(ShowBrowser((char *)globalPath, (char *)&curChosenBrowseFile[0]) == true){	//as long you keep using directories ShowBrowser will be true
				
			}
			//play TVS here
			playTVSFile((char *)&curChosenBrowseFile[0]);
			menuShow();
			while(keysDown() & KEY_START){
				scanKeys();
			}
		}
		*/
		
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
		
		/*
		if (keysDown() & KEY_LEFT){
			REG_IME = 0;
			REG_IE = 0;
			leaveTGDSLM((u32)&TGDSProjectReturnFromLinkedModuleDeciderStub);	
		}
		*/
		
		TGDSVideoRender();
	}

	return 0;
}