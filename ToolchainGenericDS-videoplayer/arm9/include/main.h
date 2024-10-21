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
#include "dsregs_asm.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern u32 * getTGDSMBV3ARM7Bootloader(); //Required by ToolchainGenericDS-multiboot v3
extern u32 * getTGDSARM7VRAMCore();	//TGDS Project specific ARM7 VRAM Core

//TGDS Dir API: Directory Iterator(s)
extern struct FileClassList * menuIteratorfileClassListCtx;			//Menu Directory Iterator
extern u32 * getTGDSMBV3ARM7Bootloader();
extern int main(int argc, char ** argv);
extern char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH];
extern char globalPath[MAX_TGDSFILENAME_LENGTH];
extern int internalCodecType;//Internal because WAV raw decompressed buffers are used if Uncompressed WAV or ADPCM
extern void setSnemulDSSpecial0xFFFF0000MPUSettings();
extern void playTVSFile(char * tvsFile);
extern void TGDSProjectReturnToCaller(char * NDSPayload);
extern char callerNDSBinary[256];
extern void menuShow();
extern char fnameRead[256];

#ifdef __cplusplus
}
#endif