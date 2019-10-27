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
//#include "fatwrapper.h"
//#include "http.h"
//#include "api68.h"

enum {
	SRC_NONE,
	SRC_WAV,
	SRC_MIKMOD,
	SRC_MP3,
	SRC_OGG,
	SRC_AAC,
	SRC_FLAC,
	SRC_SID,
	SRC_NSF,
	SRC_SPC,
	SRC_SNDH,
	SRC_GBS,
	SRC_STREAM_MP3,
	SRC_STREAM_OGG,
	SRC_STREAM_AAC
};

#define ARM9COPY 0
#define ARM7COPY 1

#define STATE_PLAYING 0
#define STATE_PAUSED 1
#define STATE_STOPPED 2
#define STATE_UNLOADED 3

// 2048 creates the crash bug in fat_fread
#define WAV_READ_SIZE 4096

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
//#include "drv_nos.h"

typedef struct 
{
	char chunkID[4];
	long chunkSize;

	short wFormatTag;
	unsigned short wChannels;
	unsigned long dwSamplesPerSec;
	unsigned long dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
} wavFormatChunk;

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

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern char *strlwr(char *str);

extern void initComplexSound();
extern void setSoundInterrupt();
extern void updateStream();

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
extern void copyChunk();
extern void setSoundFrequency(u32 freq);
extern void setSoundLength(u32 len);

extern u32 getSoundChannels();
extern u8 getVolume();
extern void setVolume(u8 volume);
extern int getStreamLag();
extern int getStreamLead();
//extern char *sidMeta(int which);

// for about screen to set or clear the ability to loop modules
extern void setLoop();
extern void clearLoop();

extern TransferSound Snd;
extern TransferSoundData SndDat;
extern void playSoundBlock(TransferSound *snd);
extern int playSound( pTransferSoundData sound);
extern int playGenericSound(const void* data, u32 length);
extern void setGenericSound( u32 rate, u8 vol, u8 pan, u8 format);
extern bool updateRequested;
extern sndData soundData;
extern void updateStream();
extern void updateStreamLoop();

#ifdef __cplusplus
}
#endif
