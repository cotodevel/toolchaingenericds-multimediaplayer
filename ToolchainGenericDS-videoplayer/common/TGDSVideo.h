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

//Format name: TVS (ToolchainGenericDS Videoplayer Stream)
//Format Version: 1.3
//Changelog:
//1.3 Add timestamp and synchronize video to audio track.
//1.2 Add LZSS compression
//1.1 add 10 FPS support
//1.0 First version, plays raw uncompressed videoframes

#ifndef __TGDSVideo_h__
#define __TGDSVideo_h__

#if defined (MSDOS) || defined(WIN32)
#define TGDSARM9Malloc malloc
#define TGDSARM9Calloc calloc
#define TGDSARM9Realloc realloc
#define TGDSARM9Free free
#endif

//enable GCC Linux and WIN32 only
#if !defined(ARM7) && !defined(ARM9)
#include "TGDSTypes.h"
#endif

#include <stdio.h>
#include <stdint.h>

#if !defined(WIN32)
#include <stdbool.h>
#endif

#ifdef ARM9
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "limitsTGDS.h"
#include "fatfslayerTGDS.h"
#include "posixHandleTGDS.h"
#include "utilsTGDS.h"
#endif

#define vblankMaxFrame (int)(60)
#define mainBufferDraw (u32)(0x06800000)
#define decodedBuf (u32)(0x06880000)
#define decompBufUncached (u32)(((int)&decompBuf[0] + 0x400000))

#define videoFramesToCache (int)(30)
#define minimumVideoFramesBeforeCaching (int)(videoFramesToCache/2)
#define invalidVideoFrameIndex (int)(-1)

#define NDS_HEIGHT (int)(192)
#define NDS_WIDTH (int)(256)

#define TGDSVideoFrameNoCompression (int)(0)
#define TGDSVideoFrameLZSSWRAMCompression (int)(1)
#define TGDSVideoFrameLZSSVRAMCompression (int)(2)
//... other compression formats

struct videoFrameTimeStamp{
	int extractedElapsedTimeStampInMilliseconds;
	int frameIndex;
};

struct videoFrame{
	int lastVideoFrameOffsetInFile; //points to previous struct videoFrame *
	int lastVideoFrameFileSize;
	int nextVideoFrameOffsetInFile; //points to next struct videoFrame *
	int nextVideoFrameFileSize;
	int elapsedTimeStampInMilliseconds; //1100ms equals to 1.1s, 100ms equals to 0.1s, etc
	int currentFrameIndex;	//TGDS-Videoplayer Client: AMMOUNT Cache buffer where up to {videoFramesToCache} will be decompressed into, using the indexed frames format. Rendered frames consume this
	int frameSize; //holds the uncompressed size
	u32 LZSSWRAMCompressedFormat; //TGDSVideoFrameNoCompression == always Uncompressed / TGDSVideoFrameLZSSWRAMCompression == always compressed
	u32 * rawVideoFrame; 	//TGDS-Videoplayer Client: Uncompressed frames will be stored here -> render to VRAM engine (allocates (NDS_HEIGHT*NDS_WIDTH*sizeof(u16)) per frame)
														//TGDS-Videoexporter PC app: compressed raw VideoFrame offset pointed in FILE handle, where {frameSize} holds the compressed size
};

#define maxVideoFramesCacheSize (int)(videoFramesToCache * (sizeof(struct videoFrame)))

struct TGDSVideoFrameContext{
	int framesPerSecond;	//Defined and emitted by TGDS-Videoexporter PC app, used by TGDS-Videoplayer Client
	int videoFramesTotalCount;	//Defined and emitted by TGDS-Videoexporter PC app, used by TGDS-Videoplayer Client.
	
	//used by TGDS-Videoplayer DS client
	FILE * TGDSVideoFileHandle;
	
	struct videoFrame * DecompressedVideoFrameQueue; 	//TGDS-Videoplayer Client: Cache buffer where up to {videoFramesToCache} will be decompressed into. 
																		//write once, then read only, from File Handle
													
													//TGDS-Videoexporter PC app: can't use, reserved by TGDSVideoPlayer client.
														
	//misc
	int frameCountUnused;	//TGDS-Videoplayer internal frame render offset. It's programmable to enable ringbuffer videoFrames
	bool TGDSVideoPlaybackUnused;
	int videoFrameStartFileOffset;	//points to first videoFrame offset in file. -1 if no videoFrames were generated or actual videoFrame offset
	int videoFrameStartFileSize;	//same, but fileSize

	u32 * frameOffsetCollection;	//Up to {videoFramesTotalCount} relative videoFrames fetched from FileHandle, using same relative index. (offsets point to physical compressed videoFrames, read as u32)
									//Note: struct videoFrame is compressed and has variable size
};

#define DMA0_SRC       (*(vuint32*)0x040000B0)
#define DMA0_DEST      (*(vuint32*)0x040000B4)
#define DMA0_CR        (*(vuint32*)0x040000B8)

#endif


#ifdef __cplusplus
extern "C" {
#endif

#if defined(ARM9) || defined(WIN32)

//NDS Client implementation
extern struct TGDSVideoFrameContext TGDSVideoFrameContextDefinition;
extern struct TGDSVideoFrameContext * TGDSVideoFrameContextReference;

extern volatile u64 DPGAudioStream_SyncSamples;
extern u32 DPGAudioStream_PregapSamples;
extern u32 vblankCount;
extern int parseTGDSVideoFile(struct fd * _VideoDecoderFileHandleFD, char * audioFname);
extern u32 getVideoFrameOffsetFromIndexInFileHandle(int videoFrameIndexFromFileHandle);

extern int TGDSVideoRender();
extern u32 frameInterval;

extern struct fd videoHandleFD;
extern FILE* audioHandleFD;

#ifdef ARM9
extern int nextVideoFrameOffset;
extern int nextVideoFrameFileSize;
#endif

extern u32 frameCount;
extern bool TGDSVideoPlayback;

extern u8 decompBuf[256*192*2];

#endif

#ifdef __cplusplus
}
#endif