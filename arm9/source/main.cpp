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
#include "fileBrowse.hpp"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.

static vector<string> songLst;
char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH+1];

#define oldSongsToRemember (int)(10)

#define SCREENWIDTH  (256)
#define SCREENHEIGHT (192)
#define COLOR(r,g,b)  ((r) | (g)<<5 | (b)<<10)
#define OFFSET(r,c,w) ((r)*(w)+(c))
#define VRAM_C            ((u16*)0x06200000)
#define PIXEL_ENABLE (1<<15)

static inline void setPixel(int row, int col, u16 color) {
	VRAM_C[OFFSET(row, col, SCREENWIDTH)] = color | PIXEL_ENABLE;
}

//What the original mandelbrot code does: (the original I based this one), as a X/Y latitude-like orientation so it will rotate around its X and Y axis, by a given factor

//Coto: What I added: 
//rendering a terrain/vortex made by the fibonnaci sequence behind the mandelbrot:

//1: each access will follow the fibonacci seq, thus colors, steps in radians, will be reciprocal by a portion of the same number
//2: use the PI number to draw circumferences of the range in colors between real and imaginary, and the current colour palette (only 5 bits of it) will be used as compass to give the circle (dist) a shift towards the right direction
//3: and then the colours generated will get sliced by the shift in 2), towards their lighter version, also, upcoming colours resemble the rainbow colors affinity. 
//4: cuadratic "textures" appearing are the result of the decreasing precision of the "palette body" while heading towards the golden number. The mandelbrot is unaffected because the formula keeps "the body" of it 1:1
static float stepsAccess = 0.001;
static inline struct rgbMandel mandelbrot(float real, float imag) {
	stepsAccess = stepsAccess + 0.002;
	int value = 31;
	float zReal = real;
	float zImag = imag;
	float dist = (((real - imag) * stepsAccess) * 3.14) / 360 * (real - imag);
	struct rgbMandel rgb = { ((31 - (dist*0.5)) / 360) * stepsAccess , ((31 - (dist*0.3)) / 360) * stepsAccess , ((31 - (dist*0.1)) / 360) * stepsAccess };
	
	for (int i = 0; i < value; ++i) {
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

static inline void draw(float x_start, float x_fin, float y_start, float y_fin) {
	
	int width = SCREENWIDTH; //number of characters fitting horizontally on my screen 
	int heigth = SCREENHEIGHT; //number of characters fitting vertically on my screen
	float dx = (x_fin - x_start)/(width-1);
	float dy = (y_fin - y_start)/(heigth-1);
	
	//stable
	for (int i = 0; i < heigth; i++) {
		float y = y_fin - i*dy; // current imaginary value
		for (int j = 0; j < width; j+=40) {
			
			float x0 = x_start + (j+0)*dx; // current real value
			float x1 = x_start + (j+1)*dx; // current real value
			float x2 = x_start + (j+2)*dx; // current real value
			float x3 = x_start + (j+3)*dx; // current real value
			float x4 = x_start + (j+4)*dx; // current real value
			float x5 = x_start + (j+5)*dx; // current real value
			float x6 = x_start + (j+6)*dx; // current real value
			float x7 = x_start + (j+7)*dx; // current real value
			float x8 = x_start + (j+8)*dx; // current real value
			float x9 = x_start + (j+9)*dx; // current real value
			
			float x10 = x_start + (j+10)*dx; // current real value
			float x11 = x_start + (j+11)*dx; // current real value
			float x12 = x_start + (j+12)*dx; // current real value
			float x13 = x_start + (j+13)*dx; // current real value
			float x14 = x_start + (j+14)*dx; // current real value
			float x15 = x_start + (j+15)*dx; // current real value
			float x16 = x_start + (j+16)*dx; // current real value
			float x17 = x_start + (j+17)*dx; // current real value
			float x18 = x_start + (j+18)*dx; // current real value
			float x19 = x_start + (j+19)*dx; // current real value
			
			float x20 = x_start + (j+20)*dx; // current real value
			float x21 = x_start + (j+21)*dx; // current real value
			float x22 = x_start + (j+22)*dx; // current real value
			float x23 = x_start + (j+23)*dx; // current real value
			float x24 = x_start + (j+24)*dx; // current real value
			float x25 = x_start + (j+25)*dx; // current real value
			float x26 = x_start + (j+26)*dx; // current real value
			float x27 = x_start + (j+27)*dx; // current real value
			float x28 = x_start + (j+28)*dx; // current real value
			float x29 = x_start + (j+29)*dx; // current real value
			
			float x30 = x_start + (j+30)*dx; // current real value
			float x31 = x_start + (j+31)*dx; // current real value
			float x32 = x_start + (j+32)*dx; // current real value
			float x33 = x_start + (j+33)*dx; // current real value
			float x34 = x_start + (j+34)*dx; // current real value
			float x35 = x_start + (j+35)*dx; // current real value
			float x36 = x_start + (j+36)*dx; // current real value
			float x37 = x_start + (j+37)*dx; // current real value
			float x38 = x_start + (j+38)*dx; // current real value
			float x39 = x_start + (j+39)*dx; // current real value
			
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
			
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 0] = (u32)((COLOR(rgb0.r, rgb0.g, rgb0.b) | PIXEL_ENABLE) | ((COLOR(rgb1.r, rgb1.g, rgb1.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 2] = (u32)((COLOR(rgb2.r, rgb2.g, rgb2.b) | PIXEL_ENABLE) | ((COLOR(rgb3.r, rgb3.g, rgb3.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 4] = (u32)((COLOR(rgb4.r, rgb4.g, rgb4.b) | PIXEL_ENABLE) | ((COLOR(rgb5.r, rgb5.g, rgb5.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 6] = (u32)((COLOR(rgb6.r, rgb6.g, rgb6.b) | PIXEL_ENABLE) | ((COLOR(rgb7.r, rgb7.g, rgb7.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 8] = (u32)((COLOR(rgb8.r, rgb8.g, rgb8.b) | PIXEL_ENABLE) | ((COLOR(rgb9.r, rgb9.g, rgb9.b) | PIXEL_ENABLE)<<16));
			
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 10] = (u32)((COLOR(rgb10.r, rgb10.g, rgb10.b) | PIXEL_ENABLE) | ((COLOR(rgb11.r, rgb11.g, rgb11.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 12] = (u32)((COLOR(rgb12.r, rgb12.g, rgb12.b) | PIXEL_ENABLE) | ((COLOR(rgb13.r, rgb13.g, rgb13.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 14] = (u32)((COLOR(rgb14.r, rgb14.g, rgb14.b) | PIXEL_ENABLE) | ((COLOR(rgb15.r, rgb15.g, rgb15.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 16] = (u32)((COLOR(rgb16.r, rgb16.g, rgb16.b) | PIXEL_ENABLE) | ((COLOR(rgb17.r, rgb17.g, rgb17.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 18] = (u32)((COLOR(rgb18.r, rgb18.g, rgb18.b) | PIXEL_ENABLE) | ((COLOR(rgb19.r, rgb19.g, rgb19.b) | PIXEL_ENABLE)<<16));
			
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 20] = (u32)((COLOR(rgb20.r, rgb20.g, rgb20.b) | PIXEL_ENABLE) | ((COLOR(rgb21.r, rgb21.g, rgb21.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 22] = (u32)((COLOR(rgb22.r, rgb22.g, rgb22.b) | PIXEL_ENABLE) | ((COLOR(rgb23.r, rgb23.g, rgb23.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 24] = (u32)((COLOR(rgb24.r, rgb24.g, rgb24.b) | PIXEL_ENABLE) | ((COLOR(rgb25.r, rgb25.g, rgb25.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 26] = (u32)((COLOR(rgb26.r, rgb26.g, rgb26.b) | PIXEL_ENABLE) | ((COLOR(rgb27.r, rgb27.g, rgb27.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 28] = (u32)((COLOR(rgb28.r, rgb28.g, rgb28.b) | PIXEL_ENABLE) | ((COLOR(rgb29.r, rgb29.g, rgb29.b) | PIXEL_ENABLE)<<16));
			
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 30] = (u32)((COLOR(rgb30.r, rgb30.g, rgb30.b) | PIXEL_ENABLE) | ((COLOR(rgb31.r, rgb31.g, rgb31.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 32] = (u32)((COLOR(rgb32.r, rgb32.g, rgb32.b) | PIXEL_ENABLE) | ((COLOR(rgb33.r, rgb33.g, rgb33.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 34] = (u32)((COLOR(rgb34.r, rgb34.g, rgb34.b) | PIXEL_ENABLE) | ((COLOR(rgb35.r, rgb35.g, rgb35.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 36] = (u32)((COLOR(rgb36.r, rgb36.g, rgb36.b) | PIXEL_ENABLE) | ((COLOR(rgb37.r, rgb37.g, rgb37.b) | PIXEL_ENABLE)<<16));
			*(u32*)&VRAM_C[(i*SCREENWIDTH) + j + 38] = (u32)((COLOR(rgb38.r, rgb38.g, rgb38.b) | PIXEL_ENABLE) | ((COLOR(rgb39.r, rgb39.g, rgb39.b) | PIXEL_ENABLE)<<16));
			
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
void drawMandel(float factor){
	float center_x = -1.04082816210546;
	float center_y = 0.3546341718848392;
	int iter = 128;
	
	float x_start = center_x - 1.5*factor;
	float x_fin = center_x + 1.5*factor;
	float y_start = center_y - factor;
	float y_fin = center_y + factor;
	draw(x_start, x_fin, y_start, y_fin);
	
	for (int i = 0; i < iter; i++) {
		factor = factor / 1.3;
		x_start = center_x - 1.5*factor;
		x_fin = center_x + 1.5*factor;
		y_start = center_y - factor;
		y_fin = center_y + factor;
		draw(x_start, x_fin, y_start, y_fin);
	}
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


static inline std::string GetFileExtension(const std::string& FileName)
{
    if(FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".")+1);
    return "";
}

bool ShowBrowser(char * Path, bool & pendingPlay){	//custom filebrowser code required by TGDS-audioplayer. Code should be a copy + update from fileBrowse.hpp
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
		//directory?
		if(retf == FT_DIR){
			struct FileClass * fileClassInst = getFileClassFromList(LastDirEntry);
			std::string outDirName = string(parseDirNameTGDS(fileClassInst->fd_namefullPath));
			strcpy(fileClassInst->fd_namefullPath, outDirName.c_str());
			internalName.push_back(fileClassInst);
		}
		//file?
		else if(retf == FT_FILE){
			struct FileClass *fileClassInst = getFileClassFromList(LastFileEntry); 
			std::string outFileName = parsefileNameTGDS(string(fileClassInst->fd_namefullPath));
			strcpy(fileClassInst->fd_namefullPath, outFileName.c_str());
			
			std::string ext = GetFileExtension(outFileName);
			if(
				(ext.compare(string("wav")) == 0)
				||
				(ext.compare(string("it")) == 0)
				||
				(ext.compare(string("mod")) == 0)
				||
				(ext.compare(string("s3m")) == 0)
				||
				(ext.compare(string("xm")) == 0)
				||
				(ext.compare(string("mp3")) == 0)
				||
				(ext.compare(string("mp2")) == 0)
				||
				(ext.compare(string("mpa")) == 0)
				||
				(ext.compare(string("ogg")) == 0)
				||
				(ext.compare(string("aac")) == 0)
				||
				(ext.compare(string("m4a")) == 0)
				||
				(ext.compare(string("m4b")) == 0)
				||
				(ext.compare(string("flac")) == 0)
				||
				(ext.compare(string("sid")) == 0)
				||
				(ext.compare(string("nsf")) == 0)
				||
				(ext.compare(string("spc")) == 0)
				||
				(ext.compare(string("sndh")) == 0)
				||
				(ext.compare(string("snd")) == 0)
				||
				(ext.compare(string("sc68")) == 0)
				||
				(ext.compare(string("gbs")) == 0)
			){
				songLst.push_back(outFileName);
			}
			
			internalName.push_back(fileClassInst);
		}
		
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
			char startPath[MAX_TGDSFILENAME_LENGTH+1];
			sprintf(startPath,"%s","/");
			while( ShowBrowser((char *)startPath, pendingPlay) == true ){	//as long you keep using directories ShowBrowser will be true
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
			float factor = 1.0; 
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