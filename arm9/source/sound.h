/***************************************************************************
 *                                                                         *
 *  This file is part of DSOrganize.                                       *
 *                                                                         *
 *  DSOrganize is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  DSOrganize is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with DSOrganize.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _SOUND_INCLUDED
#define _SOUND_INCLUDED

#include "ipcfifoTGDSUser.h"
#include <aacdec.h>
#include "misc.h"
#include "http.h"
#include "api68.h"
#include "id3.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "gui_console_connector.h"
#include "soundTGDS.h"
#include "fileBrowse.h"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.
#include "click_raw.h"
#include "global_settings.h"
#include "xmem.h"
#include "posixHandleTGDS.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ivorbiscodec.h>
#include <ivorbisfile.h>
#include <spc.h>
#include "mikmod_build.h"
#include "sid.h"
#include "nsf.h"
#include "gme.h"
#include "mixer68.h"
#include "mp4ff.h"
#include "misc.h"
#include "http.h"
#include "id3.h"
#include "ipcfifoTGDSUser.h"
#include "videoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "nds_cp15_misc.h"
#include "mad.h"
#include "flac.h"
#include "aacdec.h"
#include "main.h"
#include "soundplayerShared.h"

#define ARM9COPY 0
#define ARM7COPY 1

#define STATE_PLAYING 0
#define STATE_PAUSED 1
#define STATE_STOPPED 2
#define STATE_UNLOADED 3


#define MP3_READ_SIZE 8192
#define MP3_WRITE_SIZE 2048
#define STREAM_MP3_READ_SIZE 1024
#define STREAM_BUFFER_SIZE (1024*512) // 512 kb buffer
#define STREAM_WIFI_READ_SIZE (1024*20) // 20 kb recieve
#define STREAM_CACHE_SIZE (1024*64) // 64 kb cache size

#define OGG_READ_SIZE 1024

#define MIKMOD_FREQ 44100

#define AAC_READBUF_SIZE	(2 * AAC_MAINBUF_SIZE * AAC_MAX_NCHANS)
#define AAC_OUT_SIZE 32768
#ifdef AAC_ENABLE_SBR
  #define SBR_MUL		2
#else
  #define SBR_MUL		1
#endif

#define FLAC_OUT_SIZE 4096

#define SID_FREQ 48000
#define SID_OUT_SIZE 2048
#define SID_META_LOC 0x16

#define NSF_FREQ 48000
#define NSF_OUT_SIZE 2048

#define SPC_FREQ 32000
#define SPC_OUT_SIZE 2048

#define SNDH_OUT_SIZE 4096

#define GBS_FREQ 32000
#define GBS_OUT_SIZE 2048

// streaming stuff
#define STREAM_DISCONNECTED 0
#define STREAM_CONNECTING 1
#define STREAM_CONNECTED 2
#define STREAM_FAILEDCONNECT 3
#define STREAM_STREAMING 4
#define STREAM_STARTING 5
#define STREAM_BUFFERING 6
#define STREAM_TRYNEXT 7

#define ICY_HEADER_SIZE 2048

#define REG_SIWRAMCNT (*(vu8*)0x04000247)
#define SIWRAM0 ((s16 *)0x037F8000)
#define SIWRAM1 ((s16 *)0x037FC000)

// mikmod
#include "drv_nos.h"


typedef struct
{
	int sourceFmt;
	int bufLoc;
	int channels;
	FILE *filePointer;
	int bits;
	u32 len;
	u32 loc;
	u32 dataOffset;
	u32 dataLen;
	int mp3SampleRate;
} sndData;

// sound stuff

#define SND_GOFORWARD 0
#define SND_GOBACK 1
#define SND_GETPREV 2
#define SND_GETNEXT 3
#define SND_TOGGLEHOLD 5
#define SND_MODE 6
#define SND_VOLUMEUP 7
#define SND_VOLUMEDN 8
#define SND_PREVINTERNAL 9
#define SND_NEXTINTERNAL 10
#define SND_SEEK 11

// going to screens
#define GO_CONFIG 12

#define COMPARE_INFINITE -1

//#define SCREENSHOT_MODE
//#define MALLOC_TRACK
//#define DEBUG_MODE
//#define PROFILING

#define vblankWait() { while(REG_VCOUNT != 192); while(REG_VCOUNT == 192); }

typedef struct
{
	u32 memLocation;
	u32 size;
	char description[32];
} MALLOC_LIST;


//globals.h
#define FILENAME_SIZE 256
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 192
#define SCREEN_BPP 2
#define FULLPAGE_HEIGHT (SCREEN_HEIGHT * 2)
#define CR 0x0D
#define LF 0x0A
#define EOS 0
#define SCRATCH_START (*(void *)0x06880000)

#ifdef __cplusplus
extern "C" {
#endif

extern char *strlwr(char *str);
extern ID3V1_TYPE id3Data;
extern void setSoundInterrupt();

// playback
extern bool loadSound(char *fName);
extern void loadSoundGeneric(u32 wPlugin, int rate, int multiplicity, int sampleLength);
extern void pauseSound(bool pause);
extern void getSoundLoc(u32 *loc, u32 *max);
extern void setSoundLoc(u32 loc);
extern void closeSound();
extern void freeSound(); // should not be called unless absolutely necessary
extern int getState();
extern int getSoundLength();
extern int getSourceFmt();
extern void soundPrevTrack(int x, int y);
extern void soundNextTrack(int x, int y);
//extern int getSNDHTrack();
//extern int getSNDHTotalTracks();
//extern void getSNDHMeta(api68_music_info_t * info);
extern void loadWavToMemory();
//extern int getSIDTrack();
//extern int getSIDTotalTracks();
//extern int getGBSTrack();
//extern int getGBSTotalTracks();
//extern char *gbsMeta(int which);

// wifi
extern void SendArm7Command(u32 command, u32 data);
extern int getCurrentStatus();
//extern ICY_HEADER *getStreamData();

// for streaming record interrupt
//Audio commands: drive Sound Player Context (Note: different from soundTGDS.h -> Sound Sample Context)
extern void setSoundLengthDSO(u32 len);
extern void copyChunk();
extern void setSoundFrequency(u32 freq);

extern u32 getSoundChannels();
extern u8 getVolume();
extern void setVolume(u8 volume);
extern int getStreamLag();
extern int getStreamLead();
//extern char *sidMeta(int which);

// for about screen to set or clear the ability to loop modules
extern void setLoop();
extern void clearLoop();

extern bool updateRequested;
extern sndData soundData;
extern void checkEndSound();
extern bool soundLoaded;

// sound out
extern s16 *lBuffer;
extern s16 *rBuffer;

extern bool cutOff ;
extern bool sndPaused ;
extern bool playing ;
extern bool seekSpecial ;
extern bool updateRequested ;
extern int sndLen ;
extern int seekUpdate ;
extern ID3V1_TYPE id3Data;

extern int m_SIWRAM ;
extern int m_size ;

// mikmod
extern MODULE *module ;
extern bool madFinished ;
extern int sCursor ;
extern bool allowEQ ;

extern OggVorbis_File vf;
extern int current_section;

// aac
extern HAACDecoder *hAACDecoder;
extern unsigned char *aacReadBuf ;
extern unsigned char *aacReadPtr ;
extern s16 *aacOutBuf ;
extern AACFrameInfo aacFrameInfo;
extern int aacBytesLeft, aacRead, aacErr, aacEofReached;
extern int aacLength;
extern bool isRawAAC;
extern mp4ff_t *mp4file;
extern mp4ff_callback_t mp4cb;
extern int mp4track;
extern int sampleId;

//flac
extern FLACContext fc;
extern uint8_t *flacInBuf ;
extern int flacBLeft ;
extern int32_t *decoded0 ;
extern int32_t *decoded1 ;
extern bool flacFinished ;

extern bool isSwitching;
extern bool amountLeftOver();

#ifdef __cplusplus
}
#endif


// update function
static inline void updateStream()
{	
	if(!updateRequested)
	{
		// exit if nothing is needed
		return;
	}
	
	// clear flag for update
	updateRequested = false;
	
	if(lBuffer == NULL || rBuffer == NULL)
	{
		// file is done
		stopSound(soundData.sourceFmt);
		sndPaused = false;
		return;
	}
	
	if(sndPaused || seekSpecial)
	{
		memset(lBuffer, 0, m_size * 2);
		memset(rBuffer, 0, m_size * 2);
		
		swapAndSend(ARM7COMMAND_SOUND_COPY);
		return;
	}
	
	if(cutOff)
	{
		// file is done
		stopSound(soundData.sourceFmt);
		sndPaused = false;
		
		return;
	}
	
	//checkKeys();
	
	switch(soundData.sourceFmt)
	{
		case SRC_MIKMOD:
		{
			//checkKeys();
			if(Player_Active())
			{
				swapAndSend(ARM7COMMAND_SOUND_DEINTERLACE);
				
				if(seekUpdate >= 0)
				{
					soundData.loc = seekUpdate;
					seekUpdate = -1;
					
					Player_SetPosition(soundData.loc);
				}
				
				soundData.loc = module->sngpos;
				
				setBuffer(lBuffer);
				MikMod_Update();
			}
			else
			{
				cutOff = true;
			}
		}
		break;
		case SRC_MP3:		
		{
			swapAndSend(ARM7COMMAND_SOUND_COPY);
			
			mp3Decode();			
			soundData.loc = ftell(soundData.filePointer);
			
			if(soundData.loc > soundData.len)
				soundData.loc = soundData.len;
		}
		break;
		case SRC_STREAM_MP3:
		{
			swapAndSend(ARM7COMMAND_SOUND_COPY);	
			
			if(amountLeftOver())
				recieveStream(-1);
			
			madFinished = false;	
			
			copyRemainingData();
			
			while(!madFinished)
			{
				fillMadBufferStream(); // should take ~ 1024 bytes
				decodeMadBufferStream(1);
				//checkKeys();		
			}
			
			if(amountLeftOver())
				recieveStream(-1);
		}
		break;
		case SRC_OGG:
		case SRC_STREAM_OGG:
		{
			struct soundPlayerContext * soundPlayerCtx = soundIPC();
			soundPlayerCtx->channels = soundData.channels;
			swapAndSend(ARM7COMMAND_SOUND_DEINTERLACE);
			
			if(soundData.sourceFmt == SRC_STREAM_OGG)
			{
				if(amountLeftOver())
					recieveStream(-1);
			}
			
			u8 *readBuf = (u8 *)lBuffer;			
			int readAmount = 0;
			
			while(readAmount < OGG_READ_SIZE) // loop until we got it all
			{
				long ret;
				ret = ov_read(&vf, readBuf, ((OGG_READ_SIZE - readAmount) * 2 * soundData.channels), &current_section);
				
				if(ret == 0)
				{ 
					if(soundData.sourceFmt == SRC_OGG)
					{
						cutOff = true;
					}
					
					break;
				}
				else if (ret > 0)
				{	
					readBuf += ret;
					readAmount += ret / (2 * soundData.channels);
				}
				
				//checkKeys();
			}
			
			if(soundData.sourceFmt == SRC_STREAM_OGG)
			{
				if(amountLeftOver())
					recieveStream(-1);
			}
			else
			{
				soundData.loc = ftell(soundData.filePointer);
			}
		}
		break;
		case SRC_AAC:
		case SRC_STREAM_AAC:{
			bool isSeek = (seekUpdate >= 0);
			
			struct soundPlayerContext * soundPlayerCtx = soundIPC();
			soundPlayerCtx->channels = soundData.channels;			
			swapAndSend(ARM7COMMAND_SOUND_DEINTERLACE);
			
			if(soundData.sourceFmt == SRC_STREAM_AAC)
			{
				if(amountLeftOver())
				{
					recieveStream(-1);
				}
				
				isSeek = false;
			}
			else if(isSeek)
			{
				aacReadPtr = aacReadBuf;
				aacBytesLeft = 0;
				aacEofReached = 0;
				
				soundData.loc = seekUpdate;
				seekUpdate = -1;
				
				if(!isRawAAC)
				{
					sampleId = soundData.loc;
				}
			}	
			
			aacFillBuffer();
			aacErr = AACDecode(hAACDecoder, &aacReadPtr, &aacBytesLeft, lBuffer);
			
			if(soundData.sourceFmt == SRC_STREAM_AAC)
			{
				if(amountLeftOver())
				{
					recieveStream(-1);
				}
			}
			else
			{
				if(isRawAAC)
					soundData.loc = ftell(soundData.filePointer);
				else
					soundData.loc = sampleId;
			}
			
			switch (aacErr) 
			{
				case ERR_AAC_NONE:
					break;
				default:
					if(isSeek)
					{
						fseek(soundData.filePointer, 4, SEEK_CUR);
						seekUpdate = ftell(soundData.filePointer);
						
						memset(lBuffer, 0, m_size * 2);
						memset(rBuffer, 0, m_size * 2);
						break;
					}
					
					cutOff = true;
					
					break;
			}
		}
		break;
		case SRC_FLAC:{
			swapAndSend(ARM7COMMAND_SOUND_COPY);
			
			if(seekUpdate >= 0)
			{
				soundData.loc = seekUpdate;
				seekUpdate = -1;
				
				seekFlac();
			}
			
			flacFinished = false;
			
			//checkKeys();
			copyRemainingData();
			decodeFlacFrame();
			//checkKeys();
			
			soundData.loc = ftell(soundData.filePointer);
			
			if(soundData.loc > soundData.len)
				soundData.loc = soundData.len;
			
		}
		break;
		case SRC_SID:{
			struct soundPlayerContext * soundPlayerCtx = soundIPC();
			soundPlayerCtx->channels = 1;
			swapAndSend(ARM7COMMAND_SOUND_DEINTERLACE);
			
			//checkKeys();
			sidDecode();
			//checkKeys();
		}
		break;
		case SRC_NSF:{
			if(isSwitching)
			{
				memset(lBuffer, 0, NSF_OUT_SIZE * 4);
			}
			
			struct soundPlayerContext * soundPlayerCtx = soundIPC();
			soundPlayerCtx->channels = 1;
			swapAndSend(ARM7COMMAND_SOUND_DEINTERLACE);
			
			//checkKeys();			
			nsfDecode();
			//checkKeys();
		}
		break;
		case SRC_SPC:{
			swapAndSend(ARM7COMMAND_SOUND_COPY);
			
			//checkKeys();			
			spcDecode();
			//checkKeys();
		}
		break;
		case SRC_SNDH:{
			struct soundPlayerContext * soundPlayerCtx = soundIPC();
			soundPlayerCtx->channels = 2;
			swapAndSend(ARM7COMMAND_SOUND_DEINTERLACE);
			
			//checkKeys();			
			sndhDecode();
			//checkKeys();
		}
		break;
		case SRC_GBS:{
			struct soundPlayerContext * soundPlayerCtx = soundIPC();
			soundPlayerCtx->channels = 2;
			swapAndSend(ARM7COMMAND_SOUND_DEINTERLACE);
			
			//checkKeys();			
			gbsDecode();
			//checkKeys();
		}
		break;
	}
	
	//checkKeys();
}

#endif