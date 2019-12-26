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

#ifndef __main7_h__
#define __main7_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "ipcfifoTGDSUser.h"

static inline void TIMER1Handler()
{	
	setSwapChannel();
	SendFIFOWords(ARM9COMMAND_UPDATE_BUFFER, 0);
}

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern int main(int _argc, sint8 **_argv);
extern s16 *strpcmL0;
extern s16 *strpcmL1;
extern s16 *strpcmR0;
extern s16 *strpcmR1;

extern int lastL;
extern int lastR;

extern int multRate;
extern int pollCount; //start with a read

extern u32 sndCursor;
extern u32 micBufLoc;
extern u32 sampleLen;
extern int sndRate;

extern void mallocData(int size);
extern void freeData();
extern void setSwapChannel();
extern void SetupSound();
extern void StopSound();

#ifdef __cplusplus
}
#endif

