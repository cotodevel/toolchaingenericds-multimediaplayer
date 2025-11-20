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

#include "../include/main.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "gui/gui_console_connector.h"
#include "misc.h"
#include "sound.h"
#include "soundTGDS.h"
#include "keypadTGDS.h"
#include "biosTGDS.h"
#include "dmaTGDS.h"
#include "TGDSLogoLZSSCompressed.h"
#include "fileBrowse.h"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.
#include "utilsTGDS.h"
#include "nds_cp15_misc.h"
#include "mikmod_internals.h"
#include "fatfslayerTGDS.h"
#include "loader.h"
#include "spitscTGDS.h"
#include "timerTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "InterruptsARMCores_h.h"
#include "tgds_intro_m4a.h"
#include "dswnifi_lib.h"
#include "powerTGDS.h"
#include "TGDSMemoryAllocator.h"
#include "TGDSVideo.h"
#include "TGDS_threads.h"
#include "WoopsiTemplate.h"

//ARM7 VRAM core: *.TVS IMA-ADPCM support
#include "arm7bootldr_standalone.h"
#include "arm7bootldr_standalone_twl.h"

u32 * getTGDSMBV3ARM7AudioCore(){
	if(__dsimode == false){
		swiDecompressLZSSWram((u8*)&arm7bootldr_standalone[0], (u8*)decompBufUncached);
	}
	else{
		swiDecompressLZSSWram((u8*)&arm7bootldr_standalone_twl[0], (u8*)decompBufUncached);
	}
	return (u32*)decompBufUncached;
}

char currentFileChosen[MAX_TGDSFILENAME_LENGTH+1];
void handleInput(){
	//scanKeys(); //if enabled, destroys key button presses from WoopsiSDK
	u32 readKeys_ = keysDown();
	
	switch(readKeys_){
		case (KEY_TOUCH):{
			bottomScreenIsLit = true; //input event triggered
		}break;

		case (KEY_A):{
			bottomScreenIsLit = true; //input event triggered
			if(WoopsiTemplateProc != NULL){
				WoopsiTemplateProc->_play->ClickButton();
			}
			while(keysHeld() & KEY_A){
				scanKeys();
			}	
		}break;

		case (KEY_B):{
			bottomScreenIsLit = true; //input event triggered
			if(WoopsiTemplateProc != NULL){
				WoopsiTemplateProc->_stop->ClickButton();
			}
			while(keysHeld() & KEY_B){
				scanKeys();
			}	
		}break;

		case (KEY_UP):{
			bottomScreenIsLit = true; //input event triggered
			if(WoopsiTemplateProc != NULL){
				WoopsiTemplateProc->currentFileRequesterIndex--;
				FileRequester * freqInst = WoopsiTemplateProc->_fileReq;
				FileListBox* freqListBox = freqInst->getInternalListBoxObject();
				
				//Start scrolling up every nth items
				if( (WoopsiTemplateProc->currentFileRequesterIndex >= 0) && ((WoopsiTemplateProc->currentFileRequesterIndex % 2) == 0) ){
					freqListBox->getInternalScrollingListBoxObject()->getInternalScrollbarVerticalObject()->getInternalUpperArrowButtonObject()->ClickButton();
				}

				if(WoopsiTemplateProc->currentFileRequesterIndex < 0){
					WoopsiTemplateProc->currentFileRequesterIndex = 0;
				}
				
				freqListBox->setSelectedIndex(WoopsiTemplateProc->currentFileRequesterIndex);
				WoopsiTemplateProc->redraw();
			}
			while(keysDown() & KEY_UP){
				scanKeys();
			}	
		}break;

		case (KEY_DOWN):{
			bottomScreenIsLit = true; //input event triggered
			if(WoopsiTemplateProc != NULL){
				FileRequester * freqInst = WoopsiTemplateProc->_fileReq;
				FileListBox* freqListBox = freqInst->getInternalListBoxObject();
				int lstSize = (freqListBox->getOptionCount() - 1);
				WoopsiTemplateProc->currentFileRequesterIndex++;
				if(WoopsiTemplateProc->currentFileRequesterIndex >= lstSize){
					WoopsiTemplateProc->currentFileRequesterIndex = lstSize;
				}
				freqListBox->setSelectedIndex(WoopsiTemplateProc->currentFileRequesterIndex);
				
				//Start scrolling after 6th item
				if( (WoopsiTemplateProc->currentFileRequesterIndex > 5) && ( WoopsiTemplateProc->currentFileRequesterIndex < (freqListBox->getOptionCount() - 1))){
					freqListBox->getInternalScrollingListBoxObject()->getInternalScrollbarVerticalObject()->getInternalLowerArrowButtonObject()->ClickButton();
				}
				WoopsiTemplateProc->redraw();
			}
			while(keysDown() & KEY_DOWN){
				scanKeys();
			}
		}break;

		case (KEY_L):{
			bottomScreenIsLit = true; //input event triggered
			if(WoopsiTemplateProc != NULL){
				WoopsiTemplateProc->_lastFile->ClickButton();
			}
			while(keysDown() & KEY_L){
				scanKeys();
			}	
		}break;

		case (KEY_R):{
			bottomScreenIsLit = true; //input event triggered
			if(WoopsiTemplateProc != NULL){
				WoopsiTemplateProc->_nextFile->ClickButton();
			}
			while(keysDown() & KEY_R){
				scanKeys();
			}	
		}break;

		case (KEY_SELECT):{
			bottomScreenIsLit = true; //input event triggered
			//0 = playlist / 1 = repeat
			if(playbackMode == 1){
				playbackMode = 0;
			}
			else{
				playbackMode = 1;
			}
			
			while(keysDown() & KEY_SELECT){
				scanKeys();
			}
		}break;

	}
	
	//Audio track ended? Play next audio file
	if((pendPlay == 0) && (cutOff == true)){
		if(WoopsiTemplateProc != NULL){
			if(playbackMode == 0){
				WoopsiTemplateProc->currentFileRequesterIndex++;
			}

			FileRequester * freqInst = WoopsiTemplateProc->_fileReq;
			FileListBox* freqListBox = freqInst->getInternalListBoxObject();
			if(WoopsiTemplateProc->currentFileRequesterIndex >= (freqListBox->getOptionCount()) ){
				WoopsiTemplateProc->currentFileRequesterIndex = 0;
			}
			freqListBox->setSelectedIndex(WoopsiTemplateProc->currentFileRequesterIndex);
			
			//Let decoder close context so we can start again
			stopAudioStreamUser();
			playAudioFile();
		}
	}
}

int playbackMode = 0; //0 = playlist / 1 = repeat

void playIntro(){
	char * introFilename = "0:/tgds_intro.m4a";
	FILE * fh = fopen(introFilename, "w+");
	struct LZSSContext LZSSCtx = LZS_DecodeFromBuffer((u8*)&tgds_intro_m4a[0], (unsigned int)tgds_intro_m4a_size);
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)LZSSCtx.bufferSource, (int)LZSSCtx.bufferSize);
	int written = fwrite((u8*)LZSSCtx.bufferSource, 1, LZSSCtx.bufferSize, fh);
	fclose(fh);
	//used? discard
	TGDSARM9Free(LZSSCtx.bufferSource);

	if(written == LZSSCtx.bufferSize){
		strcpy(currentFileChosen, (const char *)introFilename);
		stopAudioFile();
		pendPlay = 1;
	}
	else{
		printf("couldn't play the intro.");
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void stopAudioStreamUser(){
	bottomScreenIsLit = true; //input event triggered
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
	pendPlay = 2; //stop filestream immediately
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
	//Save Stage 1: IWRAM ARM7 payload: NTR/TWL (0x03800000)
	memcpy((void *)TGDS_MB_V3_ARM7_STAGE1_ADDR, (const void *)0x02380000, (int)(96*1024));
	coherent_user_range_by_size((uint32)TGDS_MB_V3_ARM7_STAGE1_ADDR, (int)(96*1024));
	memcpy((void *)savedDefaultCore, (const void *)0x02380000, (int)(96*1024));
	coherent_user_range_by_size((uint32)savedDefaultCore, (int)(96*1024));
	
	//Let dlmalloc handle memory management
	extern void ds_malloc_abort(void);
	u32 * fnPtr = (u32 *)&ds_malloc_abort;
	mem_cpy((u8*)fnPtr, (u8*)&ds_malloc_abortSkip, 16);

	bool project_specific_console = false;	//set default console or custom console: custom console
	GUI_init(project_specific_console);
	GUI_clear();
	
	//default newlib-nds's malloc
	bool isCustomTGDSMalloc = false; 
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup(isCustomTGDSMalloc));
	
	//WoopsiSDK's malloc (unused)
	//bool isCustomTGDSMalloc = true; 
	//setTGDSMemoryAllocator(getWoopsiSDKToolchainGenericDSMultibootMemoryAllocatorSetup(isCustomTGDSMalloc));
	
	sint32 fwlanguage = (sint32)getLanguage();
	
	project_specific_console = true;	//set default console or custom console: custom console
	GUI_init(project_specific_console);
	GUI_clear();
	
	printf("     ");
	printf("     ");
	
	int ret=FS_init();
	if (ret != 0){
		printf("%s: FS Init error: %d >%d", TGDSPROJECTNAME, ret, TGDSPrintfColor_Red);
		while(1==1){
			swiDelay(1);
		}
	}
	
	//switch_dswnifi_mode(dswifi_idlemode);
	asm("mcr	p15, 0, r0, c7, c10, 4");
	flush_icache_all();
	flush_dcache_all();
	/*			TGDS 1.6 Standard ARM9 Init code end	*/
	
	REG_IME = 0;
	MPUSet();
	//TGDS-Projects -> legacy NTR TSC compatibility
	if(__dsimode == true){
		TWLSetTouchscreenTWLMode();
	}
	REG_IME = 1;
	
	powerOFF3DEngine(); //Power off ARM9 3D Engine to save power
	setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT); 
	bottomScreenIsLit = true;
	
	MikMod_RegisterAllDrivers();
	MikMod_RegisterAllLoaders();
	
	REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	REG_IE = REG_IE & ~(IRQ_TIMER3); //disable WIFI timer
	REG_IE = (REG_IE | IRQ_VBLANK | IRQ_VCOUNT);
	
	//Register threads.
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	int taskATimeMS = 80; //Task execution requires at least 35ms, also, thread in milliseconds will run too slow, give it the highest priority.
    if(registerThread(TGDSThreads, (TaskFn)&taskA, (u32*)NULL, taskATimeMS, (TaskFn)&onThreadOverflowUserCode, tUnitsMicroseconds) != THREAD_OVERFLOW){
        
    }

	TGDSVideoPlayback = false;
	playIntro();
	enableScreenPowerTimeout();

	// Create Woopsi UI
	WoopsiTemplate WoopsiTemplateApp;
	WoopsiTemplateProc = &WoopsiTemplateApp;
	return WoopsiTemplateApp.main(argc, argv);
}

//Skip newlib-nds's dlmalloc abort handler and let dlmalloc memory manager handle gracefully invalid memory blocks, later to be re-assigned when fragmented memory gets re-arranged as valid memory blocks.
void ds_malloc_abortSkip(void){
	
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void enableScreenPowerTimeout(){
	setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);

	//Add timeout thread here
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	int taskBTimeMS = 1; //Task execution requires at least 1ms
	if(registerThread(TGDSThreads, (TaskFn)&taskB, (u32*)NULL, taskBTimeMS, (TaskFn)&onThreadOverflowUserCode, tUnitsMilliseconds) != THREAD_OVERFLOW){
			
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void disableScreenPowerTimeout(){
	setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);

	//Remove touchscreen thread here to prevent turning off the screen while other operations like:
	//*.TVS playback takes place 
	//file list navigation
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	removeThread(TGDSThreads, (TaskFn)&taskB);
}

bool bottomScreenIsLit = false;
static int millisecondsElapsed = 0;	

//called 50 times per second
void handleTurnOnTurnOffScreenTimeout(){
	millisecondsElapsed ++;
	if (  millisecondsElapsed >= 500 ){
		setBacklight(0);
		millisecondsElapsed = 0;
	}
	//turn on bottom screen if input event
	if(bottomScreenIsLit == true){
		setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
		bottomScreenIsLit = false;
		millisecondsElapsed = 0;
	}
}


//////////////////////////////////////////////////////// Threading User code start : TGDS Project specific ////////////////////////////////////////////////////////
//User callback when Task Overflows. Intended for debugging purposes only, as normal user code tasks won't overflow if a task is implemented properly.
//	u32 * args = This Task context
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void onThreadOverflowUserCode(u32 * args){
	struct task_def * thisTask = (struct task_def *)args;
	struct task_Context * parentTaskCtx = thisTask->parentTaskCtx;	//get parent Task Context node 

	char threadStatus[64];
	switch(thisTask->taskStatus){
		case(INVAL_THREAD):{
			strcpy(threadStatus, "INVAL_THREAD");
		}break;
		
		case(THREAD_OVERFLOW):{
			strcpy(threadStatus, "THREAD_OVERFLOW");
		}break;
		
		case(THREAD_EXECUTE_OK_WAIT_FOR_SLEEP):{
			strcpy(threadStatus, "THREAD_EXECUTE_OK_WAIT_FOR_SLEEP");
		}break;
		
		case(THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE):{
			strcpy(threadStatus, "THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE");
		}break;
	}
	
	char debOut2[256];
	char timerUnitsMeasurement[32];
	if( thisTask->taskStatus == THREAD_OVERFLOW){
		if(thisTask->timerFormat == tUnitsMilliseconds){
			strcpy(timerUnitsMeasurement, "ms");
		}
		else if(thisTask->timerFormat == tUnitsMicroseconds){
			strcpy(timerUnitsMeasurement, "us");
		} 
		else{
			strcpy(timerUnitsMeasurement, "-");
		}
		sprintf(debOut2, "[%s]. Thread requires at least (%d) %s. ", threadStatus, thisTask->remainingThreadTime, timerUnitsMeasurement);
	}
	else{
		sprintf(debOut2, "[%s]. ", threadStatus);
	}
	
	int TGDSDebuggerStage = 10;
	u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
	handleDSInitOutputMessage((char*)debOut2);
	handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
	
	while(1==1){
		HaltUntilIRQ();
	}
}

__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void taskA(u32 * args){
	//Audio playback here....
	
	switch(soundData.sourceFmt){
		case(SRC_VGM):{
			updateStream();
		}break;

		default:{
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
		}break;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void taskB(u32 * args){
	handleTurnOnTurnOffScreenTimeout();
}