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
#include "TGDSLogoLZSSCompressed.h"
#include "fileBrowse.h"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.
#include "biosTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "dldi.h"
#include "global_settings.h"
#include "posixHandleTGDS.h"
#include "TGDSMemoryAllocator.h"
#include "consoleTGDS.h"
#include "sound.h"
#include "soundTGDS.h"
#include "nds_cp15_misc.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "click_raw.h"
#include "ima_adpcm.h"
#include "misc.h"

// Includes
#include "WoopsiTemplate.h"
#define SCREENWIDTH  (256)
#define SCREENHEIGHT (192)
#define COLOR(r,g,b)  ((r) | (g)<<5 | (b)<<10)
#define OFFSET(r,c,w) ((r)*(w)+(c))
#define VRAM_A            ((u16*)0x06000000)
#define PIXEL_ENABLE (1<<15)

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
struct rgbMandel mandelbrot(float real, float imag) {
	stepsAccess = stepsAccess + 0.002;
	int value = 31;
	float zReal = real;
	float zImag = imag;
	float dist = (((real - imag) * stepsAccess) * 3.14) / 360 * (real - imag);
	struct rgbMandel rgb = { (int)(((unsigned int)((31 - (dist*0.5)) / 360)) * stepsAccess), (int)(((unsigned int)((31 - (dist*0.3)) / 360)) * stepsAccess), (int)(((unsigned int)((31 - (dist*0.1)) / 360)) * stepsAccess) };
	
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


static inline void menuShow(){
	clrscr();
	printf("     ");
	printf("     ");
	printf("toolchaingenericds-audioplayer: ");
	printf("(Select): This menu. ");
	printf("(Start): FileBrowser : (A) Play WAV/IMA-ADPCM (Intel) strm ");
	printf("(D-PAD:UP/DOWN): Volume + / - ");
	printf("(D-PAD:LEFT): GDB Debugging. >%d", TGDSPrintfColor_Green);
	printf("(D-PAD:RIGHT): Demo Sound. >%d", TGDSPrintfColor_Yellow);
	printf("(B): Stop WAV/IMA-ADPCM file. ");
	printf("Current Volume: %d", (int)getVolume());
	if(internalCodecType == SRC_WAVADPCM){
		printf("ADPCM Play: >%d", TGDSPrintfColor_Red);
	}
	else if(internalCodecType == SRC_WAV){	
		printf("WAVPCM Play: >%d", TGDSPrintfColor_Green);
	}
	else{
		printf("Player Inactive");
	}
	printf("Available heap memory: %d >%d", getMaxRam(), TGDSPrintfColor_Cyan);
	printarm7DebugBuffer();
}


//ToolchainGenericDS-LinkedModule User implementation: Called if TGDS-LinkedModule fails to reload ARM9.bin from DLDI.
char args[8][MAX_TGDSFILENAME_LENGTH];
char *argvs[8];
int TGDSProjectReturnFromLinkedModule() __attribute__ ((optnone)) {
	return -1;
}

int main(int argc, char **argv) __attribute__ ((optnone)) {
	/*			TGDS 1.6 Standard ARM9 Init code start	*/
	bool isTGDSCustomConsole = true;	//set default console or custom console: default console
	GUI_init(isTGDSCustomConsole);
	GUI_clear();
	
	printf("              ");
	printf("              ");
	
	bool isCustomTGDSMalloc = true;
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, isCustomTGDSMalloc));
	sint32 fwlanguage = (sint32)getLanguage();
	
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
	asm("mcr	p15, 0, r0, c7, c10, 4");
	flush_icache_all();
	flush_dcache_all();
	/*			TGDS 1.6 Standard ARM9 Init code end	*/
	
	//Show logo
	RenderTGDSLogoMainEngine((uint8*)&TGDSLogoLZSSCompressed[0], TGDSLogoLZSSCompressed_size);
	MikMod_RegisterAllDrivers();
	MikMod_RegisterAllLoaders();

	// Create Woopsi UI
	WoopsiTemplate WoopsiTemplateApp;
	WoopsiTemplateProc = &WoopsiTemplateApp;
	return WoopsiTemplateApp.main(argc, argv);
	
	while(1) {
		handleARM9SVC();	/* Do not remove, handles TGDS services */
		IRQWait(IRQ_HBLANK);
	}

	return 0;
}

