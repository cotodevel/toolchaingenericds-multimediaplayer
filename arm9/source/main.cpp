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

//ARM7 VRAM core: *.TVS IMA-ADPCM support
#include "arm7bootldr_standalone.h"
#include "arm7bootldr_standalone_twl.h"

u32 * getTGDSMBV3ARM7AudioCore(){
	if(__dsimode == false){
		return (u32*)&arm7bootldr_standalone[0];	
	}
	else{
		return (u32*)&arm7bootldr_standalone_twl[0];
	}
}

//TGDS Dir API: Directory Iterator(s)
struct FileClassList * playListRead = NULL;			//Menu Directory Iterator
struct FileClassList * activePlayListRead = NULL;				//Playlist Directory Iterator

//static vector<string> songLst;
char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH+1];
char globalPath[MAX_TGDSFILENAME_LENGTH+1];

#define oldSongsToRemember (int)(10)

bool keypadLocked=false;
static bool drawMandelbrt = false;
bool pendingPlay = false;
int curFileIndex = 0;
int lastRand = 0;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
bool ShowBrowserC(char * Path, char * outBuf, bool * pendingPlay, int * curFileIndex){	//MUST be same as the template one at "fileBrowse.h" but added some custom code
	scanKeys();
	while((keysDown() & KEY_START) || (keysDown() & KEY_A) || (keysDown() & KEY_B)){
		bottomScreenIsLit = true; //input event triggered
		scanKeys();
	}
	
	//Create TGDS Dir API context
	cleanFileList(playListRead);
	
	//Use TGDS Dir API context
	int pressed = 0;
	int j = 0;
	int startFromIndex = 1;
	*curFileIndex = startFromIndex;
	
	//Generate an active playlist
	readDirectoryIntoFileClass(Path, playListRead);
	cleanFileList(activePlayListRead);
	int itemsFound = buildFileClassByExtensionFromList(playListRead, activePlayListRead, (char**)ARM7_PAYLOAD, (char*)"/ima/tvs/wav/it/mod/s3m/xm/mp3/mp2/mpa/ogg/aac/m4a/m4b/flac/sid/nsf/spc/sndh/snd/sc68/gbs");
	activePlayListRead->FileDirCount--; //skipping the first item pushed before
	
	//Sort list alphabetically
	bool ignoreFirstFileClass = true;
	sortFileClassListAsc(playListRead, (char**)ARM7_PAYLOAD, ignoreFirstFileClass);
	
	//actual file lister
	clrscr();
	
	int lastVal = 0;
	bool reloadDirA = false;
	bool reloadDirB = false;
	char * newDir = NULL;
	
	#define itemsShown (int)(15)
	int curjoffset = 0;
	int itemRead=0;
	int printYoffset = 1;
	
	while(1){
		int fileClassListSize = getCurrentDirectoryCount(activePlayListRead) + 1;
		int itemsToLoad = (fileClassListSize - curjoffset); 
		
		//check if remaining items are enough
		if(itemsToLoad > itemsShown){
			itemsToLoad = itemsShown;
		}
		
		while(itemRead < itemsToLoad ){		
			if(getFileClassFromList(itemRead+curjoffset, activePlayListRead)->type == FT_DIR){
				printfCoords(0, itemRead + printYoffset, "--- %s >%d",getFileClassFromList(itemRead+curjoffset, activePlayListRead)->fd_namefullPath, TGDSPrintfColor_Yellow);
			}
			else if(getFileClassFromList(itemRead+curjoffset, activePlayListRead)->type == FT_FILE){
				printfCoords(0, itemRead + printYoffset, "--- %s",getFileClassFromList(itemRead+curjoffset, activePlayListRead)->fd_namefullPath);
			}
			else{
				//itemRead--;
				//itemsToLoad--;
			}
			itemRead++;
		}
		
		scanKeys();
		pressed = keysDown();
		if (pressed&KEY_DOWN && (j < (itemsToLoad - 1) ) ){
			bottomScreenIsLit = true; //input event triggered
			j++;
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysDown();
			}
		}
		
		//downwards: means we need to reload new screen
		else if(pressed&KEY_DOWN && (j >= (itemsToLoad - 1) ) && ((fileClassListSize - curjoffset - itemRead) > 0) ){
			bottomScreenIsLit = true; //input event triggered
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1 - printYoffset);
			itemRead = 0;
			j = 0;
			
			scanKeys();
			pressed = keysDown();
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysDown();
			}
		}
		
		//LEFT, reload new screen
		else if(pressed&KEY_LEFT && ((curjoffset - itemsToLoad) > 0) ){
			bottomScreenIsLit = true; //input event triggered
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset - itemsToLoad - 1 - printYoffset);
			itemRead = 0;
			j = 0;
			
			scanKeys();
			pressed = keysDown();
			while(pressed&KEY_LEFT){
				scanKeys();
				pressed = keysDown();
			}
		}
		
		//RIGHT, reload new screen
		else if(pressed&KEY_RIGHT && ((fileClassListSize - curjoffset - itemsToLoad) > 0) ){
			bottomScreenIsLit = true; //input event triggered
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1 - printYoffset);
			itemRead = 0;
			j = 0;
			
			scanKeys();
			pressed = keysDown();
			while(pressed&KEY_RIGHT){
				scanKeys();
				pressed = keysDown();
			}
		}
		
		else if (pressed&KEY_UP && (j >= 1)) {
			bottomScreenIsLit = true; //input event triggered
			j--;
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysDown();
			}
		}
		
		//upwards: means we need to reload new screen
		else if (pressed&KEY_UP && (j <= 1) && (curjoffset > 0) ) {
			bottomScreenIsLit = true; //input event triggered
			//list only the remaining items
			clrscr();
			
			curjoffset--;
			itemRead = 0;
			j = 0;
			
			scanKeys();
			pressed = keysDown();
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysDown();
			}
		}
		
		//reload DIR (forward)
		else if( (pressed&KEY_A) && (getFileClassFromList(j+curjoffset, activePlayListRead)->type == FT_DIR) ){
			struct FileClass * fileClassChosen = getFileClassFromList(j+curjoffset, activePlayListRead);
			newDir = fileClassChosen->fd_namefullPath;
			reloadDirA = true;
			break;
		}
		
		//file chosen
		else if( (pressed&KEY_A) && (getFileClassFromList(j+curjoffset, activePlayListRead)->type == FT_FILE) ){
			break;
		}
		
		//reload DIR (backward)
		else if(pressed&KEY_B){
			reloadDirB = true;
			break;
		}
		
		//Handle normal input to turn back on bottom screen 
		if(
			(pressed&KEY_TOUCH)
			||
			(pressed&KEY_A)
			||
			(pressed&KEY_B)
			||
			(pressed&KEY_X)
			||
			(pressed&KEY_Y)
			||
			(pressed&KEY_UP)
			||
			(pressed&KEY_DOWN)
			||
			(pressed&KEY_LEFT)
			||
			(pressed&KEY_RIGHT)
			||
			(pressed&KEY_L)
			||
			(pressed&KEY_R)
			||
			(pressed&KEY_SELECT)
			||
			(pressed&KEY_START)
			){
			bottomScreenIsLit = true; //input event triggered
		}

		// Show cursor
		printfCoords(0, j + printYoffset, "*");
		if(lastVal != j){
			
			//clean old
			if(getFileClassFromList(j+curjoffset, activePlayListRead)->type == FT_FILE){
				printfCoords(0, lastVal, "---");	
			}
			else if(getFileClassFromList(j+curjoffset, activePlayListRead)->type == FT_DIR){
				printfCoords(0, lastVal, "--->%d", TGDSPrintfColor_Yellow);
			}
			else if(getFileClassFromList(j+curjoffset, activePlayListRead)->type == FT_NONE){
				printfCoords(0, lastVal, "   ");
			}
			
		}
		lastVal = j + printYoffset;
	}
	
	//enter a dir
	if(reloadDirA == true){
		//Free TGDS Dir API context
		//freeFileList(activePlayListRead);	//can't because we keep the activePlayListRead handle across folders
		
		enterDir((char*)newDir, Path);
		return true;
	}
	
	//leave a dir
	if(reloadDirB == true){
		//Free TGDS Dir API context
		//freeFileList(activePlayListRead);	//can't because we keep the activePlayListRead handle across folders
		
		//rewind to preceding dir in TGDSCurrentWorkingDirectory
		leaveDir(Path);
		return true;
	}
	
	strcpy((char*)outBuf, getFileClassFromList(j+curjoffset, activePlayListRead)->fd_namefullPath);
	clrscr();
	printf("                                   ");
	if(getFileClassFromList(j+curjoffset, activePlayListRead)->type == FT_DIR){
		//printf("you chose Dir:%s",outBuf);
	}
	else if(getFileClassFromList(j+curjoffset, activePlayListRead)->type == FT_FILE){
		*curFileIndex = (j+curjoffset);	//Update Current index in the playlist
		*pendingPlay = true;
	}
	
	//Free TGDS Dir API context
	//freeFileList(activePlayListRead);
	return false;
}

void handleInput(){
	if(pendingPlay == true){
		soundLoaded = loadSound((char*)curChosenBrowseFile);
		pendingPlay = false;
		menuShow();
		drawMandelbrt = false;
	}
	
	scanKeys();
	if (keysDown() & KEY_A){
		bottomScreenIsLit = true; //input event triggered
		keypadLocked=!keypadLocked;
		menuShow();
		while(keysDown() & KEY_A){
			scanKeys();
		}
	}

	if(keypadLocked == false){
		if (keysDown() & KEY_UP){
			bottomScreenIsLit = true; //input event triggered
			struct touchPosition touchPos;
			XYReadScrPosUser(&touchPos);
			volumeUp(touchPos.px, touchPos.py);
			menuShow();
			scanKeys();
			while(keysDown() & KEY_UP){
				scanKeys();
			}
		}
		
		if (keysDown() & KEY_DOWN){
			bottomScreenIsLit = true; //input event triggered
			struct touchPosition touchPos;
			XYReadScrPosUser(&touchPos);
			volumeDown(touchPos.px, touchPos.py);
			menuShow();
			scanKeys();
			while(keysDown() & KEY_DOWN){
				scanKeys();
			}
		}
		
		
		if (keysDown() & KEY_TOUCH){
			bottomScreenIsLit = true; //input event triggered
			u8 channel = 0;	//-1 == auto allocate any channel in the 0--15 range
			//setSoundSampleContext(11025, (u32*)&click_raw[0], click_raw_size, channel, 40, 63, 1);	//PCM16 sample //todo: use writeARM7SoundChannelFromSource() instead
			scanKeys();
			while(keysDown() & KEY_TOUCH){
				scanKeys();
			}
		}
		
		if (keysDown() & KEY_L){
			bottomScreenIsLit = true; //input event triggered
			soundPrevTrack(0, 0);
			scanKeys();
			while(keysHeld() & KEY_L){
				scanKeys();
			}
			menuShow();
		}
		
		if (keysDown() & KEY_R){
			bottomScreenIsLit = true; //input event triggered
			soundNextTrack(0, 0);
			scanKeys();
			while(keysHeld() & KEY_R){
				scanKeys();
			}
			menuShow();
		}
		
		if (keysDown() & KEY_START){
			bottomScreenIsLit = true; //input event triggered
			if(soundLoaded == false){
				while( ShowBrowserC((char *)globalPath, curChosenBrowseFile, &pendingPlay, &curFileIndex) == true ){	//as long you keep using directories ShowBrowser will be true
					//navigating DIRs here...
				}
				scanKeys();
				while(keysDown() & KEY_START){
					scanKeys();
				}
				
				//handle TVS files only when pressing Start
				char tmpName[256];
				char ext[256];
				char bootldr[256];
				strcpy(tmpName, curChosenBrowseFile);
				separateExtension(tmpName, ext);
				strlwr(ext);
				
				//TGDS-MB + TGDS-videoplayer TVS file
				if(strncmp(ext,".tvs", 4) == 0){
					playTVSFile(curChosenBrowseFile);
				}
			}
			else{
				clrscr();
				printfCoords(0, 6, "Please stop audio playback before listing files. ");
				
				scanKeys();
				while(keysDown() & KEY_START){
					scanKeys();
				}
				menuShow();
			}
			
		}
		
		if (keysDown() & KEY_B){
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
			pendingPlay = false;

			menuShow();
			
			scanKeys();
			while(keysDown() & KEY_B){
				scanKeys();
			}
		}
		
		//Coto: this piece of art is magic, truth.
		/*
		if (keysDown() & KEY_X){
			bottomScreenIsLit = true; //input event triggered
			if(drawMandelbrt == false){
				drawMandelbrt = true;
				double factor = 1.0; 
				drawMandel(factor);
				//render TGDSLogo from a LZSS compressed file
				RenderTGDSLogoMainEngine((uint8*)&TGDSLogoLZSSCompressed[0], TGDSLogoLZSSCompressed_size);
			}
			
			scanKeys();
			while(keysDown() & KEY_X){
				scanKeys();
			}
		}
		*/

		if (keysDown() & KEY_SELECT){
			bottomScreenIsLit = true; //input event triggered
			//0 = playlist / 1 = repeat
			if(playbackMode == 1){
				playbackMode = 0;
			}
			else{
				playbackMode = 1;
			}
			menuShow();
			scanKeys();
			while(keysDown() & KEY_SELECT){
				scanKeys();
			}
		}

	}

	//Audio track ended? Play next audio file
	if((pendingPlay == false) && (cutOff == true)){ 
		
		if(playbackMode == 0){
			curFileIndex++;
		}

		if(curFileIndex >= getCurrentDirectoryCount(activePlayListRead)){
			curFileIndex = 0;
		}
		struct FileClass * Inst = getFileClassFromList(curFileIndex, activePlayListRead);
		if(Inst != NULL){
			strcpy(curChosenBrowseFile, (const char *)Inst->fd_namefullPath);
			
			//Let decoder close context so we can start again
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
			pendingPlay = true;
		}
	}
}

int playbackMode = 0; //0 = playlist / 1 = repeat

void menuShow(){
	clrscr();
	printf("                              ");
	printf("%s >%d", TGDSPROJECTNAME, TGDSPrintfColor_Yellow);
	printf("Free Mem : %d KB ", ( (int)TGDSARM9MallocFreeMemory()/1024) );
	printf("Formats: ");
	printf("IMA-ADPCM (Intel)/WAV/MP3/AAC/Ogg >%d", TGDSPrintfColor_Yellow);
	printf("/FLAC/NSF/SPC/GBS/.TVS VideoStream >%d", TGDSPrintfColor_Yellow);
	printf("(Start): File Browser -> (A) to play audio file");
	printf("(L): Recent Playlist ");
	printf("(R): Random audio file playback ");
	
	if(keypadLocked == false){
		printf("(A): Keys [Unlocked] >%d", TGDSPrintfColor_Green);
	}
	else{
		printf("(A): Keys [Locked] >%d", TGDSPrintfColor_Red);
	}

	printf("(B): Stop audio playback ");
	//printf("(X): Mandelbrot demo ");
	printf("(D-PAD: Down): Volume - ");
	printf("(D-PAD: Up): Volume + ");
	printf("(Select): Playback Mode");
	if(soundLoaded == false){
		printf("Playback: Stopped.");
	}
	else{
		
		if(soundData.sourceFmt == SRC_GBS){
			printf("Playing: %s[%d/%d]", gbsMeta(0), getGBSTrack(), getGBSTotalTracks());
		}
		else if(soundData.sourceFmt == SRC_NSF){
			printf("Playing: %s[%d/%d]", getNSFMeta(0), getNSFTrack(), getNSFTotalTracks());
		}
		else{
			printf("Playing: %s", curChosenBrowseFile);
		}
	}
	printf("Current Volume: %d", (int)getVolume());

	if(playbackMode == 0){
		printf("Playback mode: Playlist");
	}
	else if(playbackMode == 1){
		printf("Playback mode: Repeat ");
	}
	else{
		printf("Unhandled Playback mode");
	}
}

void playIntro(){
	char * introFilename = "0:/tgds_intro.m4a";
	FILE * fh = fopen(introFilename, "w+");
	int written = fwrite((u8*)&tgds_intro_m4a[0], 1, tgds_intro_m4a_size, fh);
	fclose(fh);
	
	if(written == tgds_intro_m4a_size){
		//Create TGDS Dir API context
		cleanFileList(playListRead);
		cleanFileList(activePlayListRead);
		
		readDirectoryIntoFileClass("/", activePlayListRead);
		readDirectoryIntoFileClass("/", playListRead);
		curFileIndex = -1;
		strcpy(curChosenBrowseFile, (const char *)introFilename);
		
		//Let decoder close context so we can start again
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
		
		pendingPlay = true;
	}
	else{
		printf("couldn't play the intro.");
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
	//Save Stage 1: IWRAM ARM7 payload: NTR/TWL (0x03800000)
	memcpy((void *)TGDS_MB_V3_ARM7_STAGE1_ADDR, (const void *)0x02380000, (int)(96*1024));
	coherent_user_range_by_size((uint32)TGDS_MB_V3_ARM7_STAGE1_ADDR, (int)(96*1024));
	memcpy((void *)savedDefaultCore, (const void *)0x02380000, (int)(96*1024));
	coherent_user_range_by_size((uint32)savedDefaultCore, (int)(96*1024));
	
	bool project_specific_console = false;	//set default console or custom console: custom console
	GUI_init(project_specific_console);
	GUI_clear();
	
	bool isCustomTGDSMalloc = true; //default newlib-nds's malloc
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup(isCustomTGDSMalloc));
	sint32 fwlanguage = (sint32)getLanguage();
	
	project_specific_console = false;	//set default console or custom console: custom console
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
	
	//render TGDSLogo from a LZSS compressed file
	RenderTGDSLogoMainEngine((uint8*)&TGDSLogoLZSSCompressed[0], TGDSLogoLZSSCompressed_size);
	
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

	//Init TGDS FS Directory Iterator Context(s). Mandatory to init them like this!! Otherwise several functions won't work correctly.
	playListRead = initFileList();
	activePlayListRead = initFileList();
	
	memset(globalPath, 0, sizeof(globalPath));
	strcpy(globalPath,"/");
	
	MikMod_RegisterAllDrivers();
	MikMod_RegisterAllLoaders();
	
	REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	REG_IE = REG_IE & ~(IRQ_TIMER3|IRQ_VCOUNT); //disable VCOUNT and WIFI timer
	REG_IE = (REG_IE | IRQ_VBLANK);
	
	//Register threads.
	int taskATimeMS = 35; //Task execution requires at least 35us 
    initThreadSystem(&threadQueue);
	
	//Thread in milliseconds will run too slow, give it the highest priority.
    if(registerThread(&threadQueue, (TaskFn)&taskA, (u32*)NULL, taskATimeMS, (TaskFn)&onThreadOverflowUserCode, tUnitsMicroseconds) != THREAD_OVERFLOW){
        
    }

	int taskBTimeMS = 1; //Task execution in unit * milliseconds 
    if(registerThread(&threadQueue, (TaskFn)&taskB, (u32*)NULL, taskBTimeMS, (TaskFn)&onThreadOverflowUserCode, tUnitsMilliseconds) != THREAD_OVERFLOW){
        
    }

	keypadLocked=false;
	TGDSVideoPlayback = false;
	menuShow();
	playIntro();
	enableFastMode();
	
	while (1){
		if(TGDSVideoPlayback == true){
			TGDSVideoRender();
		}
		else{
			handleInput();
		}
		int threadsRan = runThreads(&threadQueue);
	}
	
	return 0;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void enableScreenPowerTimeout(){
	setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void disableScreenPowerTimeout(){
	setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
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