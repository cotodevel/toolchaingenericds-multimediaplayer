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

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern int main(int _argc, sint8 **_argv);
extern char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH+1];
extern void handleInput();

#ifdef __cplusplus
}
#endif