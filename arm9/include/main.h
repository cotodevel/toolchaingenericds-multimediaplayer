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

#ifndef __main9_h__
#define __main9_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "limitsTGDS.h"

struct rgbMandel{
	int r;
	int g;
	int b;
};

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern u32 * getTGDSMBV3ARM7AudioCore();
extern int main(int argc, char **argv);
extern char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH+1];
extern char globalPath[MAX_TGDSFILENAME_LENGTH+1];
extern void drawMandel(double factor);
extern void menuShow();

//TGDS Dir API: Directory Iterator(s)
extern struct FileClassList * playListRead;			//Internal playlist required by active playlist
extern struct FileClassList * activePlayListRead;	//active playlist, actual playlist

extern bool ShowBrowserC(char * Path, char * outBuf, bool * pendingPlay, int * curFileIndex);
extern void handleInput();
extern void setPixel(int row, int col, u16 color);
extern struct rgbMandel mandelbrot(float real, float imag);
extern bool keypadLocked;
extern void playIntro();

extern bool pendingPlay;
extern int curFileIndex;
extern int lastRand;
extern void handleTurnOnTurnOffScreenTimeout();
extern bool bottomScreenIsLit;
extern int playbackMode;
extern void enableScreenPowerTimeout();
extern void disableScreenPowerTimeout();

extern void taskA(u32 * args);
extern void taskB(u32 * args);
extern void onThreadOverflowUserCode(u32 * args);

#ifdef __cplusplus
}
#endif