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

//ARM7 VRAM core
#include "arm7bootldr_standalone.h"
#include "arm7bootldr_standalone_twl.h"

u32 * getTGDSMBV3ARM7Bootloader(){
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

#define SCREENWIDTH  (256)
#define SCREENHEIGHT (192)
#define COLOR(r,g,b)  ((r) | (g)<<5 | (b)<<10)
#define OFFSET(r,c,w) ((r)*(w)+(c))
#define VRAM_A            ((u16*)0x06000000)
#define PIXEL_ENABLE (1<<15)

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("Ofast")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setPixel(int row, int col, u16 color) {
	VRAM_A[OFFSET(row, col, SCREENWIDTH)] = color | PIXEL_ENABLE;
}

//What the original mandelbrot code does: (the original I based this one), as a X/Y latitude-like orientation so it will rotate around its X and Y axis, by a given factor

//Coto: What I added: 
//rendering a terrain/vortex made by the fibonnaci sequence behind the mandelbrot:

//1: each access will follow the fibonacci seq, thus colors, steps in radians, will be reciprocal by a portion of the same number
//2: use the PI number to draw circumferences of the range in colors between real and imaginary, and the current colour palette (only 5 bits of it) will be used as compass to give the circle (dist) a shift towards the right direction
//3: and then the colours generated will get sliced by the shift in 2), towards their lighter version, also, upcoming colours resemble the rainbow colors affinity. 
//4: cuadratic "textures" appearing are the result of the decreasing precision of the "palette body" while heading towards the golden number. The mandelbrot is unaffected because the formula keeps "the body" of it 1:1
static float stepsAccess = 0.001;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
struct rgbMandel mandelbrot(float real, float imag) {
	stepsAccess = stepsAccess + 0.002;
	int value = 31;
	float zReal = real;
	float zImag = imag;
	float dist = (((real - imag) * stepsAccess) * 3.14) / 360 * (real - imag);
	struct rgbMandel rgb = { ((31 - (dist*0.5)) / 360) * stepsAccess , ((31 - (dist*0.3)) / 360) * stepsAccess , ((31 - (dist*0.1)) / 360) * stepsAccess };
	
	int i = 0;
	for (i = 0; i < value; ++i) {
		float r2 = zReal * zReal;
		float i2 = zImag * zImag;
		if (r2 + i2 > 4.0) { 
			value = i;
			break;
		}
		zImag = 2.0 * zReal * zImag + imag;
		zReal = r2 - i2 + real;
	}
	if (value >= 30) {
		rgb.b = rgb.r - dist;
		rgb.g = rgb.b - dist + rgb.b;
		rgb.b = rgb.g - dist + rgb.r;
	}
	else if (value >= 29) {
		rgb.g = 8;
		rgb.b = 3;
	}
	
	else if (value >= 28) {
		rgb.g = 24;
		rgb.b = 1;
	}
	
	else if (value >= 27) {
		rgb.g = 2;
		rgb.b = 11;
	}
	
	else if (value >= 26) {
		rgb.g = 19;
		rgb.b = 19;
	}
	
	else if (value >= 25) {
		rgb.g = 12;
		rgb.b = 12;
	}
	else if (value >= 24) {
		rgb.r = 30;
		rgb.g = 10;
		rgb.b = 10;
	}
	else if (value >= 23) {
		rgb.r = 31;
		rgb.g = 11;
		rgb.b = 2;
	}
	else if (value >= 22) {
		rgb.r = 31;
		rgb.g = 31;
		rgb.b = 0;
	}
	else if (value >= 21) {
		rgb.r = 27;
		rgb.g = 31;
		rgb.b = 19;
	}
	else if (value >= 20) {
		rgb.g = 31;
	}
	else if (value >= 19) {
		rgb.r = 25;
		rgb.g = 31;
		rgb.b = 31;
	}
	else if (value >= 18) {
		rgb.r = 0;
		rgb.g = 31;
		rgb.b = 31;
	}
	else if (value >= 17) {
		rgb.r = 21;
		rgb.g = 26;
		rgb.b = 28;
	}
	else if (value >= 16) {
		rgb.b = 31;
	}
	else if (value >= 15) {
		rgb.r = 31;
		rgb.g = 0;
		rgb.b = 31;
	}
	else if (value >= 14) {
		rgb.r = 21;
		rgb.g = 10;
		rgb.b = 11;
	}
	else if (value >= 13) {
		rgb.r = 11;
		rgb.g = 9;
		rgb.b = 1;
	}
	else if (value >= 12) {
		rgb.r = 0;
		rgb.g = 29;
		rgb.b = 31;
	}
	else if (value >= 11) {
		rgb.r = 1;
		rgb.g = 1;
		rgb.b = 31;
	}
	else if (value >= 10) {
		rgb.r = 10;
		rgb.g = 1;
		rgb.b = 16;
	}
	else if (value >= 9) {
		rgb.r = 1;
		rgb.g = 29;
		rgb.b = 21;
	}
	else if (value >= 8) {
		rgb.r = 11;
		rgb.g = 9;
		rgb.b = 1;
	}
	else if (value <= 7) {
		rgb.r = 10 - dist;
		rgb.g = rgb.r + rgb.g;
		rgb.b = 0;
	}
	
	else if (value <= 6) {
		rgb.r = 10 - dist;
		rgb.g = rgb.r + rgb.g;
		rgb.b = 0;
	}
	
	else if (value <= 5) {
		rgb.g = 10 - dist;
		rgb.b = rgb.r + rgb.g;
		rgb.r = 0;
	}
	
	else if (value <= 3) {
		rgb.g = rgb.r + rgb.b;
		rgb.b = rgb.g - dist;
		rgb.r = rgb.b - dist;
	}
	
	else {
		rgb.g = 0;
		rgb.b = 0;
		rgb.r = 31;
	}
	return rgb;
}

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

static inline int getRand(int size){
	int res = (( (rand() + (REG_VCOUNT&(256-1))) ) % size);
	if(res >= 0){
		return res;
	}
	return getRand(size);
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
					strcpy(tmpName, curChosenBrowseFile);
					/////////////////////////////////////////////////////////////////////////////////////////////////////////////
					char startPath[MAX_TGDSFILENAME_LENGTH+1];
					strcpy(startPath,"/");
					if(__dsimode == true){
						strcpy(bootldr, "0:/ToolchainGenericDS-videoplayer.srl");
					}
					else{
						strcpy(bootldr, "0:/ToolchainGenericDS-videoplayer.nds");
					}
					//Send args
					int argcCount = 0;
					argcCount++;
					printf("[Booting... Please wait] >%d", TGDSPrintfColor_Red);
					
					char thisArgv[3][MAX_TGDSFILENAME_LENGTH];
					memset(thisArgv, 0, sizeof(thisArgv));
					strcpy(&thisArgv[0][0], TGDSPROJECTNAME);		//Arg0:	This Binary loaded
					strcpy(&thisArgv[1][0], bootldr);				//Arg1:	NDS Binary reloaded
					strcpy(&thisArgv[2][0], curChosenBrowseFile);	//Arg2: NDS Binary ARG0
					u32 * payload = getTGDSMBV3ARM7Bootloader();
					if(TGDSMultibootRunNDSPayload(bootldr, (u8*)payload, 3, (char*)&thisArgv) == false){ //should never reach here, nor even return true. Should fail it returns false
						printf("Invalid NDS/TWL Binary >%d", TGDSPrintfColor_Yellow);
						printf("or you are in NTR mode trying to load a TWL binary. >%d", TGDSPrintfColor_Yellow);
						printf("or you are missing the TGDS-multiboot payload in root path. >%d", TGDSPrintfColor_Yellow);
						printf("Press (A) to continue. >%d", TGDSPrintfColor_Yellow);
						while(1==1){
							scanKeys();
							if(keysDown()&KEY_A){
								bottomScreenIsLit = true; //input event triggered
								scanKeys();
								while(keysDown() & KEY_A){
									scanKeys();
								}
								break;
							}
						}
						menuShow();
					}
					/////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

static u32 bufDraw[80/4];

static inline void draw(double x_start, double x_fin, double y_start, double y_fin) {
	
	int width = SCREENWIDTH; //number of characters fitting horizontally on my screen 
	int heigth = SCREENHEIGHT; //number of characters fitting vertically on my screen
	double dx = (x_fin - x_start)/(width-1);
	double dy = (y_fin - y_start)/(heigth-1);
	int i = 0;
	for (i = 0; i < heigth; i++) {
		double y = y_fin - i*dy; // current imaginary value
		int j = 0;
		for (j = 0; j < width; j+=40) {
			
			double x0 = x_start + (j+0)*dx; // current real value
			double x1 = x_start + (j+1)*dx; // current real value
			double x2 = x_start + (j+2)*dx; // current real value
			double x3 = x_start + (j+3)*dx; // current real value
			double x4 = x_start + (j+4)*dx; // current real value
			double x5 = x_start + (j+5)*dx; // current real value
			double x6 = x_start + (j+6)*dx; // current real value
			double x7 = x_start + (j+7)*dx; // current real value
			double x8 = x_start + (j+8)*dx; // current real value
			double x9 = x_start + (j+9)*dx; // current real value
			
			double x10 = x_start + (j+10)*dx; // current real value
			double x11 = x_start + (j+11)*dx; // current real value
			double x12 = x_start + (j+12)*dx; // current real value
			double x13 = x_start + (j+13)*dx; // current real value
			double x14 = x_start + (j+14)*dx; // current real value
			double x15 = x_start + (j+15)*dx; // current real value
			double x16 = x_start + (j+16)*dx; // current real value
			double x17 = x_start + (j+17)*dx; // current real value
			double x18 = x_start + (j+18)*dx; // current real value
			double x19 = x_start + (j+19)*dx; // current real value
			
			double x20 = x_start + (j+20)*dx; // current real value
			double x21 = x_start + (j+21)*dx; // current real value
			double x22 = x_start + (j+22)*dx; // current real value
			double x23 = x_start + (j+23)*dx; // current real value
			double x24 = x_start + (j+24)*dx; // current real value
			double x25 = x_start + (j+25)*dx; // current real value
			double x26 = x_start + (j+26)*dx; // current real value
			double x27 = x_start + (j+27)*dx; // current real value
			double x28 = x_start + (j+28)*dx; // current real value
			double x29 = x_start + (j+29)*dx; // current real value
			
			double x30 = x_start + (j+30)*dx; // current real value
			double x31 = x_start + (j+31)*dx; // current real value
			double x32 = x_start + (j+32)*dx; // current real value
			double x33 = x_start + (j+33)*dx; // current real value
			double x34 = x_start + (j+34)*dx; // current real value
			double x35 = x_start + (j+35)*dx; // current real value
			double x36 = x_start + (j+36)*dx; // current real value
			double x37 = x_start + (j+37)*dx; // current real value
			double x38 = x_start + (j+38)*dx; // current real value
			double x39 = x_start + (j+39)*dx; // current real value
			
			struct rgbMandel rgb0 = mandelbrot(x0,y);	
			struct rgbMandel rgb1 = mandelbrot(x1,y);	
			struct rgbMandel rgb2 = mandelbrot(x2,y);	
			struct rgbMandel rgb3 = mandelbrot(x3,y);	
			struct rgbMandel rgb4 = mandelbrot(x4,y);	
			struct rgbMandel rgb5 = mandelbrot(x5,y);	
			struct rgbMandel rgb6 = mandelbrot(x6,y);	
			struct rgbMandel rgb7 = mandelbrot(x7,y);	
			struct rgbMandel rgb8 = mandelbrot(x8,y);	
			struct rgbMandel rgb9 = mandelbrot(x9,y);	
			
			struct rgbMandel rgb10 = mandelbrot(x10,y);	
			struct rgbMandel rgb11 = mandelbrot(x11,y);	
			struct rgbMandel rgb12 = mandelbrot(x12,y);	
			struct rgbMandel rgb13 = mandelbrot(x13,y);	
			struct rgbMandel rgb14 = mandelbrot(x14,y);	
			struct rgbMandel rgb15 = mandelbrot(x15,y);	
			struct rgbMandel rgb16 = mandelbrot(x16,y);	
			struct rgbMandel rgb17 = mandelbrot(x17,y);	
			struct rgbMandel rgb18 = mandelbrot(x18,y);	
			struct rgbMandel rgb19 = mandelbrot(x19,y);
			
			struct rgbMandel rgb20 = mandelbrot(x20,y);	
			struct rgbMandel rgb21 = mandelbrot(x21,y);	
			struct rgbMandel rgb22 = mandelbrot(x22,y);	
			struct rgbMandel rgb23 = mandelbrot(x23,y);	
			struct rgbMandel rgb24 = mandelbrot(x24,y);	
			struct rgbMandel rgb25 = mandelbrot(x25,y);	
			struct rgbMandel rgb26 = mandelbrot(x26,y);	
			struct rgbMandel rgb27 = mandelbrot(x27,y);	
			struct rgbMandel rgb28 = mandelbrot(x28,y);	
			struct rgbMandel rgb29 = mandelbrot(x29,y);
			
			struct rgbMandel rgb30 = mandelbrot(x30,y);	
			struct rgbMandel rgb31 = mandelbrot(x31,y);	
			struct rgbMandel rgb32 = mandelbrot(x32,y);	
			struct rgbMandel rgb33 = mandelbrot(x33,y);	
			struct rgbMandel rgb34 = mandelbrot(x34,y);	
			struct rgbMandel rgb35 = mandelbrot(x35,y);	
			struct rgbMandel rgb36 = mandelbrot(x36,y);	
			struct rgbMandel rgb37 = mandelbrot(x37,y);	
			struct rgbMandel rgb38 = mandelbrot(x38,y);	
			struct rgbMandel rgb39 = mandelbrot(x39,y);
			
			bufDraw[0] = (u32)((COLOR(rgb0.r, rgb0.g, rgb0.b) | PIXEL_ENABLE) | ((COLOR(rgb1.r, rgb1.g, rgb1.b) | PIXEL_ENABLE)<<16));
			bufDraw[1] = (u32)((COLOR(rgb2.r, rgb2.g, rgb2.b) | PIXEL_ENABLE) | ((COLOR(rgb3.r, rgb3.g, rgb3.b) | PIXEL_ENABLE)<<16));
			bufDraw[2] = (u32)((COLOR(rgb4.r, rgb4.g, rgb4.b) | PIXEL_ENABLE) | ((COLOR(rgb5.r, rgb5.g, rgb5.b) | PIXEL_ENABLE)<<16));
			bufDraw[3] = (u32)((COLOR(rgb6.r, rgb6.g, rgb6.b) | PIXEL_ENABLE) | ((COLOR(rgb7.r, rgb7.g, rgb7.b) | PIXEL_ENABLE)<<16));
			bufDraw[4] = (u32)((COLOR(rgb8.r, rgb8.g, rgb8.b) | PIXEL_ENABLE) | ((COLOR(rgb9.r, rgb9.g, rgb9.b) | PIXEL_ENABLE)<<16));
			bufDraw[5] = (u32)((COLOR(rgb10.r, rgb10.g, rgb10.b) | PIXEL_ENABLE) | ((COLOR(rgb11.r, rgb11.g, rgb11.b) | PIXEL_ENABLE)<<16));
			bufDraw[6] = (u32)((COLOR(rgb12.r, rgb12.g, rgb12.b) | PIXEL_ENABLE) | ((COLOR(rgb13.r, rgb13.g, rgb13.b) | PIXEL_ENABLE)<<16));
			bufDraw[7] = (u32)((COLOR(rgb14.r, rgb14.g, rgb14.b) | PIXEL_ENABLE) | ((COLOR(rgb15.r, rgb15.g, rgb15.b) | PIXEL_ENABLE)<<16));
			bufDraw[8] = (u32)((COLOR(rgb16.r, rgb16.g, rgb16.b) | PIXEL_ENABLE) | ((COLOR(rgb17.r, rgb17.g, rgb17.b) | PIXEL_ENABLE)<<16));
			bufDraw[9] = (u32)((COLOR(rgb18.r, rgb18.g, rgb18.b) | PIXEL_ENABLE) | ((COLOR(rgb19.r, rgb19.g, rgb19.b) | PIXEL_ENABLE)<<16));
			bufDraw[10] = (u32)((COLOR(rgb20.r, rgb20.g, rgb20.b) | PIXEL_ENABLE) | ((COLOR(rgb21.r, rgb21.g, rgb21.b) | PIXEL_ENABLE)<<16));
			bufDraw[11] = (u32)((COLOR(rgb22.r, rgb22.g, rgb22.b) | PIXEL_ENABLE) | ((COLOR(rgb23.r, rgb23.g, rgb23.b) | PIXEL_ENABLE)<<16));
			bufDraw[12] = (u32)((COLOR(rgb24.r, rgb24.g, rgb24.b) | PIXEL_ENABLE) | ((COLOR(rgb25.r, rgb25.g, rgb25.b) | PIXEL_ENABLE)<<16));
			bufDraw[13] = (u32)((COLOR(rgb26.r, rgb26.g, rgb26.b) | PIXEL_ENABLE) | ((COLOR(rgb27.r, rgb27.g, rgb27.b) | PIXEL_ENABLE)<<16));
			bufDraw[14] = (u32)((COLOR(rgb28.r, rgb28.g, rgb28.b) | PIXEL_ENABLE) | ((COLOR(rgb29.r, rgb29.g, rgb29.b) | PIXEL_ENABLE)<<16));
			bufDraw[15] = (u32)((COLOR(rgb30.r, rgb30.g, rgb30.b) | PIXEL_ENABLE) | ((COLOR(rgb31.r, rgb31.g, rgb31.b) | PIXEL_ENABLE)<<16));
			bufDraw[16] = (u32)((COLOR(rgb32.r, rgb32.g, rgb32.b) | PIXEL_ENABLE) | ((COLOR(rgb33.r, rgb33.g, rgb33.b) | PIXEL_ENABLE)<<16));
			bufDraw[17] = (u32)((COLOR(rgb34.r, rgb34.g, rgb34.b) | PIXEL_ENABLE) | ((COLOR(rgb35.r, rgb35.g, rgb35.b) | PIXEL_ENABLE)<<16));
			bufDraw[18] = (u32)((COLOR(rgb36.r, rgb36.g, rgb36.b) | PIXEL_ENABLE) | ((COLOR(rgb37.r, rgb37.g, rgb37.b) | PIXEL_ENABLE)<<16));
			bufDraw[19] = (u32)((COLOR(rgb38.r, rgb38.g, rgb38.b) | PIXEL_ENABLE) | ((COLOR(rgb39.r, rgb39.g, rgb39.b) | PIXEL_ENABLE)<<16));
			coherent_user_range_by_size((uint32)&bufDraw[0], 80);
			dmaTransferWord(0, (u32)&bufDraw[0], (u32)&VRAM_A[(i*SCREENWIDTH) + j], (uint32)80);
			
		} // width == 256
		//handleInput();
	}
	
}


__attribute__((section(".itcm")))
void drawMandel(double factor){
	double center_x = -1.04082816210546;
	double center_y = 0.3546341718848392;
	int iter = 128;
	
	double x_start = center_x - 1.5*factor;
	double x_fin = center_x + 1.5*factor;
	double y_start = center_y - factor;
	double y_fin = center_y + factor;
	draw(x_start, x_fin, y_start, y_fin);
	
	int i = 0;
	for (i = 0; i < iter; i++) {
		factor = factor / 1.3;
		x_start = center_x - 1.5*factor;
		x_fin = center_x + 1.5*factor;
		y_start = center_y - factor;
		y_fin = center_y + factor;
		draw(x_start, x_fin, y_start, y_fin);
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
	memcpy((void *)TGDS_MB_V3_ARM7_STAGE1_ADDR, (const void *)0x02380000, (int)(96*1024));	//
	coherent_user_range_by_size((uint32)TGDS_MB_V3_ARM7_STAGE1_ADDR, (int)(96*1024)); //		also for TWL binaries 
	
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
	
	//VBLANK can't be used to count up screen power timeout because sound stutters. Use timer instead
	TIMERXDATA(2) = TIMER_FREQ((int)1);
	TIMERXCNT(2) = TIMER_DIV_1 | TIMER_IRQ_REQ | TIMER_ENABLE;
	irqEnable(IRQ_TIMER2);

	keypadLocked=false;
	menuShow();
	playIntro();
	enableFastMode();
	
	while (1){	
		handleInput();
		
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
		
		HaltUntilIRQ(); //Save power until next irq
	}
	
	return 0;
}

bool bottomScreenIsLit = false;

static int secondsElapsed = 0;
void handleTurnOnTurnOffScreenTimeout(){
	secondsElapsed ++;

	if (  secondsElapsed == 12300 ){ //2728hz per unit @ 33Mhz
		setBacklight(0);
		secondsElapsed = 0;
	}

	//turn on bottom screen if input event
	if(bottomScreenIsLit == true){
		setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT);
		bottomScreenIsLit = false;
		secondsElapsed = 0;
	}
}