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
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern int main(int argc, char ** argv);
//TGDS Dir API: Directory Iterator(s)
extern struct FileClassList * menuIteratorfileClassListCtx;			//Menu Directory Iterator
extern char curChosenBrowseFile[256+1];
extern char globalPath[MAX_TGDSFILENAME_LENGTH+1];
extern int internalCodecType;//Internal because WAV raw decompressed buffers are used if Uncompressed WAV or ADPCM

extern char args[8][MAX_TGDSFILENAME_LENGTH];
extern char *argvs[8];
extern void leaveTGDSLM(u32 returnAddress);

extern int textureID;
extern float rotateX;
extern float rotateY;
extern float camDist;
extern char args[8][MAX_TGDSFILENAME_LENGTH];
extern char *argvs[8];
extern void setSnemulDSSpecial0xFFFF0000MPUSettings();
extern void playTVSFile(char * tvsFile);
extern void TGDSProjectReturnToCaller(char * NDSPayload);
extern char callerNDSBinary[256];
extern void menuShow();

#ifdef __cplusplus
}
#endif