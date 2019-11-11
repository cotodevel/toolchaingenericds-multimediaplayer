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
#include "dsregs_asm.h"

#include "gui_console_connector.h"
#include "misc.h"
#include "sound.h"
#include "dswnifi_lib.h"
#include "TGDSNDSLogo.h"

//C++ part
using namespace std;
#include <string>
#include <vector>
#include <algorithm>

#define oldSongsToRemember (int)(10)

static vector<string> songLst;
char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH+1];

string ToStr( char c ) {
   return string( 1, c );
}

#define SCREENWIDTH  (256)
#define SCREENHEIGHT (192)
#define COLOR(r,g,b)  ((r) | (g)<<5 | (b)<<10)

static inline void setPixel(int row, int col, u16 color) {
    #define OFFSET(r,c,w) ((r)*(w)+(c))
	#define VRAM_C            ((u16*)0x06200000)
	#define PIXEL_ENABLE (1<<15)
	VRAM_C[OFFSET(row, col, SCREENWIDTH)] = color | PIXEL_ENABLE;
}

static inline int mandelbrot(double real, double imag) {
	int limit = 100;
	double zReal = real;
	double zImag = imag;

	for (int i = 0; i < limit; ++i) {
		double r2 = zReal * zReal;
		double i2 = zImag * zImag;
		
		if (r2 + i2 > 4.0) return i;

		zImag = 2.0 * zReal * zImag + imag;
		zReal = r2 - i2 + real;
	}
	return limit;
}

static inline void draw(double x_start, double x_fin, double y_start, double y_fin) {
	
	int width = SCREENWIDTH; //number of characters fitting horizontally on my screen 
	int heigth = SCREENHEIGHT; //number of characters fitting vertically on my screen
	double dx = (x_fin - x_start)/(width-1);
	double dy = (y_fin - y_start)/(heigth-1);
	
	for (int i = 0; i < heigth; i++) {
		for (int j = 0; j < width; j++) {
			double x = x_start + j*dx; // current real value
			double y = y_fin - i*dy; // current imaginary value
			
			int value = mandelbrot(x,y);
			int r = 0;
			int g = 0;
			int b = 0;
			
			if (value == 100) { //cout << " ";
			}
			else if (value >= 99) {
				//cout << red << char_;
				r = 30;
			}
			else if (value >= 98) {
				//cout << l_red << char_;
				r = 30;
				g = 10;
				b = 10;
			}
			else if (value >= 96) {
				//cout << orange << char_;
				r = 31;
				g = 11;
				b = 2;
			}
			else if (value >= 94) {
				//cout << yellow << char_;
				r = 31;
				g = 31;
				b = 0;
			}
			else if (value >= 92) {
				//cout << l_green << char_;
				r = 27;
				g = 31;
				b = 19;
			}
			else if (value >= 90) {
				//cout << green << char_;
				g = 31;
			}
			else if (value >= 85) {
				//cout << l_cyan << char_;
				r = 25;
				g = 31;
				b = 31;
			}
			else if (value >= 80) {
				//cout << cyan << char_;
				r = 0;
				g = 31;
				b = 31;
			}
			else if (value >= 75) {
				//cout << l_blue << char_;
				r = 21;
				g = 26;
				b = 28;
			}
			else if (value >= 70) {
				//cout << blue << char_;
				b = 31;
			}
			else if (value >= 60) {
				//cout << magenta << char_;
				r = 31;
				g = 0;
				b = 31;
			}
			else {
				//cout << l_magenta << char_;
				r = 31;
				g = 16;
				b = 31;
			}
			VRAM_C[OFFSET(i, j, SCREENWIDTH)] = (u16)(COLOR(r,g,b) | PIXEL_ENABLE);
		} // width == 256
		handleInput();
	}	
}

static inline int getRand(int size){
	return (rand() % size);
}

static vector<string> oldSongLst;
static bool pendingPlay = false;
static int lastRand = 0;

__attribute__((section(".itcm")))
void drawMandel(double factor){
	double center_x = -1.04082816210546;
	double center_y = 0.3546341718848392;
	int iter = 4;
	int color_threshold = 25;
	
	double x_start = center_x - 1.5*factor;
	double x_fin = center_x + 1.5*factor;
	double y_start = center_y - factor;
	double y_fin = center_y + factor;

	//draw(x_start, x_fin, y_start, y_fin);
	
	for (int i = 0; i < iter; i++) {
		factor = factor / 1.3;
	
		x_start = center_x - 1.5*factor;
		x_fin = center_x + 1.5*factor;
		y_start = center_y - factor;
		y_fin = center_y + factor;
		
		if (i<color_threshold) {
			//cout << "\033[2J\033[1;1H";
			draw(x_start, x_fin, y_start, y_fin);
			//std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		else {
			//cout << "\033[2J\033[1;1H";
			//draw_deep(x_start, x_fin, y_start, y_fin);
			//std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}

	factor = factor / 1.5;
	
	x_start = center_x - 1.5*factor;
	x_fin = center_x + 1.5*factor;
	y_start = center_y - factor;
	y_fin = center_y + factor;
	//cout << "\033[2J\033[1;1H";
	//draw_deep(x_start, x_fin, y_start, y_fin);
}

void menuShow(){
	clrscr();
	printf("                              ");
	printf("Supported Formats: WAV/MP3/AAC/Ogg");
	printf("/FLAC/NSF/SPC/GBS/+ others");
	printf("                              ");
	printf("(Start): File Browser -> (A) to play audio file");
	printf("(L): Recent Playlist ");
	printf("(R): Random audio file playback ");
	printf("(B): Stop audio playback ");
	printf("(X): Mandelbrot demo ");
	printf("(Select): this menu");
	if(soundLoaded == false){
		printf("Playback: Stopped.");
	}
	else{
		printf("Playing: %s", curChosenBrowseFile);
	}
}

std::string parseDirNameTGDS(std::string dirName){
	if ((dirName.at(0) == '/') && (dirName.at(1) == '/')) {
		dirName.erase(0,1);	//trim the starting / if it has one
	}
	dirName.erase(dirName.length());	//trim the leading "/"
	return dirName;
}

std::string parsefileNameTGDS(std::string fileName){
	if ((fileName.at(2) == '/') && (fileName.at(3) == '/')) {
		fileName.erase(2,2);	//trim the starting // if it has one (since getfspath appends 0:/)
		if(fileName.at(2) != '/'){	//if we trimmed by accident the only leading / such as 0:filename instead of 0:/filename, restore it so it becomes the latter
			fileName.insert(2, ToStr('/') );
		}
	}
	return fileName;
}

bool ShowBrowser(char * Path, bool & pendingPlay){
	while((keysPressed() & KEY_START) || (keysPressed() & KEY_A) || (keysPressed() & KEY_B)){
		scanKeys();
		IRQWait(IRQ_HBLANK);
	}
	int pressed = 0;
	vector<struct FileClass *> internalName;	//required to handle FILE/DIR types from TGDS FS quick and easy
	struct FileClass filStub;
	char fname[256];
	sprintf(fname,"%s",Path);
	int j = 1;
    
	//OK, use the new CWD and build the playlist
	songLst.clear();
	internalName.push_back(&filStub);
	
	int retf = FAT_FindFirstFile(fname);
	while(retf != FT_NONE){
		struct FileClass * fileClassInst = NULL;
		//directory?
		if(retf == FT_DIR){
			fileClassInst = getFileClassFromList(LastDirEntry);
			std::string outDirName = string(fileClassInst->fd_namefullPath);
			sprintf(fileClassInst->fd_namefullPath,"%s",parseDirNameTGDS(outDirName).c_str());
		}
		//file?
		else if(retf == FT_FILE){
			fileClassInst = getFileClassFromList(LastFileEntry); 
			std::string outFileName = string(fileClassInst->fd_namefullPath);
			sprintf(fileClassInst->fd_namefullPath,"%s",parsefileNameTGDS(outFileName).c_str());
			
			//current playlist only allows known audio formats
			char tmpName[256+1];
			char ext[256+1];
			
			strcpy(tmpName, fname);	
			separateExtension(tmpName, ext);
			strlwr(ext);
			
			if(
				(strcmp(ext,".wav") == 0)
				||
				(strcmp(ext,".it") == 0)
				||
				(strcmp(ext,".mod") == 0)
				||
				(strcmp(ext,".s3m") == 0)
				||
				(strcmp(ext,".xm") == 0)
				||
				(strcmp(ext,".mp3") == 0)
				||
				(strcmp(ext,".mp2") == 0)
				||
				(strcmp(ext,".mpa") == 0)
				||
				(strcmp(ext,".ogg") == 0)
				||
				(strcmp(ext,".aac") == 0)
				||
				(strcmp(ext,".m4a") == 0)
				||
				(strcmp(ext,".m4b") == 0)
				||
				(strcmp(ext,".flac") == 0)
				||
				(strcmp(ext,".sid") == 0)
				||
				(strcmp(ext,".nsf") == 0)
				||
				(strcmp(ext,".spc") == 0)
				||
				(strcmp(ext,".sndh") == 0)
				||
				(strcmp(ext,".snd") == 0)
				||
				(strcmp(ext,".sc68") == 0)
				||
				(strcmp(ext,".gbs") == 0)
			){
				songLst.push_back(string(fileClassInst->fd_namefullPath));
			}
		}
		internalName.push_back(fileClassInst);
		
		//more file/dir objects?
		retf = FAT_FindNextFile(fname);
	}
	
	//actual file lister
	clrscr();
	
	j = 1;
	pressed = 0 ;
	int lastVal = 0;
	bool reloadDirA = false;
	bool reloadDirB = false;
	std::string newDir = std::string("");
	
	#define itemsShown (int)(15)
	int curjoffset = 0;
	int itemRead=1;
	
	while(1){
		
		int itemsToLoad = (internalName.size() - curjoffset);
		
		//check if remaining items are enough
		if(itemsToLoad > itemsShown){
			itemsToLoad = itemsShown;
		}
		
		while(itemRead < itemsToLoad ){
			std::string strDirFileName = string(internalName.at(itemRead+curjoffset)->fd_namefullPath);		
			if(internalName.at(itemRead+curjoffset)->type == FT_DIR){
				printfCoords(0, itemRead, "--- %s%s",strDirFileName.c_str(),"<dir>");
			}
			else{
				printfCoords(0, itemRead, "--- %s",strDirFileName.c_str());
			}
			itemRead++;
		}
		
		scanKeys();
		pressed = keysPressed();
		if (pressed&KEY_DOWN && (j < (itemsToLoad - 1) ) ){
			j++;
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_HBLANK);
			}
		}
		
		//downwards: means we need to reload new screen
		else if(pressed&KEY_DOWN && (j >= (itemsToLoad - 1) ) && ((internalName.size() - curjoffset - itemRead) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_HBLANK);
			}
		}
		
		//LEFT, reload new screen
		else if(pressed&KEY_LEFT && ((curjoffset - itemsToLoad) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset - itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_LEFT){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_HBLANK);
			}
		}
		
		//RIGHT, reload new screen
		else if(pressed&KEY_RIGHT && ((internalName.size() - curjoffset - itemsToLoad) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_RIGHT){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_HBLANK);
			}
		}
		
		else if (pressed&KEY_UP && (j > 1)) {
			j--;
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_HBLANK);
			}
		}
		
		//upwards: means we need to reload new screen
		else if (pressed&KEY_UP && (j <= 1) && (curjoffset > 0) ) {
			//list only the remaining items
			clrscr();
			
			curjoffset--;
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_HBLANK);
			}
		}
		
		//reload DIR (forward)
		else if( (pressed&KEY_A) && (internalName.at(j+curjoffset)->type == FT_DIR) ){
			newDir = string(internalName.at(j+curjoffset)->fd_namefullPath);
			reloadDirA = true;
			break;
		}
		
		//file chosen
		else if( (pressed&KEY_A) && (internalName.at(j+curjoffset)->type == FT_FILE) ){
			break;
		}
		
		//reload DIR (backward)
		else if(pressed&KEY_B){
			reloadDirB = true;
			break;
		}
		
		// Show cursor
		printfCoords(0, j, "*");
		if(lastVal != j){
			printfCoords(0, lastVal, " ");	//clean old
		}
		lastVal = j;
	}
	
	//enter a dir
	if(reloadDirA == true){
		internalName.clear();
		enterDir((char*)newDir.c_str());
		return true;
	}
	
	//leave a dir
	if(reloadDirB == true){
		//rewind to preceding dir in TGDSCurrentWorkingDirectory
		leaveDir(getTGDSCurrentWorkingDirectory());
		return true;
	}
	
	sprintf((char*)curChosenBrowseFile,"%s",internalName.at(j+curjoffset)->fd_namefullPath);
	clrscr();
	printf("                                   ");
	if(internalName.at(j+curjoffset)->type == FT_DIR){
		//printf("you chose Dir:%s",curChosenBrowseFile);
	}
	else{
		pendingPlay = true;
	}
	return false;
}

static bool drawMandelbrt = false;
__attribute__((section(".itcm")))	
void handleInput(){
	if(pendingPlay == true){
		soundLoaded = loadSound((char*)curChosenBrowseFile);
		pendingPlay = false;
		menuShow();
		drawMandelbrt = false;
	}
	
	scanKeys();
	
	if (keysPressed() & KEY_L){
		int oldLstSize = oldSongLst.size();
		if(oldLstSize > 0){
			strcpy(curChosenBrowseFile, (const char *)oldSongLst.at(oldLstSize - 1).c_str());
			oldSongLst.pop_back();
			
			//remember the old song's index
			std::vector<string>::iterator it = std::find(oldSongLst.begin(), oldSongLst.end(), string(curChosenBrowseFile));
			int index = std::distance(oldSongLst.begin(), it);
			lastRand = index;
			
			pendingPlay = true;
		}
		else{
			clrscr();
			printfCoords(0, 6, "No audio files in recent playlist. Play some first. ");
			printfCoords(0, 7, "Press (A).");
			
			scanKeys();
			while(!(keysPressed() & KEY_A)){
				scanKeys();
				IRQWait(IRQ_HBLANK);
			}
			menuShow();
		}
		scanKeys();
		while(keysPressed() & KEY_L){
			scanKeys();
			IRQWait(IRQ_HBLANK);
		}
	}
	
	if (keysPressed() & KEY_START){
		
		if(soundLoaded == false){
			//as long you keep using directories ShowBrowser will be true
			char startPath[MAX_TGDSFILENAME_LENGTH+1];
			sprintf(startPath,"%s","/");
			while( ShowBrowser((char *)startPath, pendingPlay) == true ){
				//navigating DIRs here...
			}
			
			scanKeys();
			while(keysPressed() & KEY_START){
				scanKeys();
				IRQWait(IRQ_HBLANK);
			}
		}
		else{
			clrscr();
			printfCoords(0, 6, "Please stop audio playback before listing files. ");
			printfCoords(0, 7, "Press (A).");
			
			scanKeys();
			while(!(keysPressed() & KEY_A)){
				scanKeys();
				IRQWait(IRQ_HBLANK);
			}
			menuShow();
		}
		
	}
	
	if (keysPressed() & KEY_B){
		//Audio stop here....
		closeSound();
		
		menuShow();
		
		scanKeys();
		while(keysPressed() & KEY_B){
			scanKeys();
			IRQWait(IRQ_HBLANK);
		}
	}
	
	if (keysPressed() & KEY_X){
		if(drawMandelbrt == false){
			drawMandelbrt = true;
			double factor = 1.0; 
			drawMandel(factor);
			renderFBMode3SubEngine((u16*)&TGDSLogoNDSSize[0], (int)TGDSLOGONDSSIZE_WIDTH,(int)TGDSLOGONDSSIZE_HEIGHT);
		}
		
		scanKeys();
		while(keysPressed() & KEY_X){
			scanKeys();
			IRQWait(IRQ_HBLANK);
		}
	}
	
	if (keysPressed() & KEY_R){
		//Play Random song from current folder
		int lstSize = songLst.size();
		if(lstSize > 0){
			closeSound();
			
			//pick one and play
			int randFile = -1;
			while( (randFile = getRand(lstSize)) == lastRand){
				if(lstSize == 1){
					break;	//means rand() will loop forever here because the random number will always be 0
				}
			}
			
			//remember playlist as long the audio file is unique. (for L button)
			int oldPlsSize = oldSongLst.size();
			if( (oldPlsSize > 0) && ((oldSongLst.at(oldPlsSize -1).compare(string(curChosenBrowseFile))) != 0) ){
				oldSongLst.push_back(string(curChosenBrowseFile));
			}
			else if (oldPlsSize == 0){
				oldSongLst.push_back(string(curChosenBrowseFile));
			}
			
			strcpy(curChosenBrowseFile, (const char *)songLst.at(randFile).c_str());
			pendingPlay = true;
			
			scanKeys();
			while(keysPressed() & KEY_R){
				scanKeys();
				IRQWait(IRQ_HBLANK);
			}
			lastRand = randFile;
		}
	}
	
	//Audio playback here....
	updateStreamLoop();	//runs once per hblank line
	
}

__attribute__((section(".itcm")))
int main(int _argc, sint8 **_argv) {
	
	/*			TGDS 1.5 Standard ARM9 Init code start	*/
	bool project_specific_console = true;	//set default console or custom console: custom console
	GUI_init(project_specific_console);
	GUI_clear();
	
	sint32 fwlanguage = (sint32)getLanguage();
	
	printf("     ");
	printf("     ");
	
	int ret=FS_init();
	if (ret == 0)
	{
		printf("FS Init ok.");
	}
	else if(ret == -1)
	{
		printf("FS Init error.");
	}
	switch_dswnifi_mode(dswifi_idlemode);
	/*			TGDS 1.5 Standard ARM9 Init code end	*/
	
	//show TGDS logo
	initFBModeSubEngine0x06200000();
	renderFBMode3SubEngine((u16*)&TGDSLogoNDSSize[0], (int)TGDSLOGONDSSIZE_WIDTH,(int)TGDSLOGONDSSIZE_HEIGHT);

	//Init sound
	disableVBlank();
	setGenericSound(11025, 127, 64, 1);
	initComplexSound(); // initialize sound variables
	oldSongLst.clear();
	
	menuShow();
	

	while (1){
		handleInput();
		IRQWait(IRQ_HBLANK);
	}
	
	return 0;
}