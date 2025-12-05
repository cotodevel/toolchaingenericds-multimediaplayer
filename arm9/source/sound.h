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

#include "../../common/ipcfifoTGDSUser.h"
#include "aac/pub/aacdec.h"
#include "misc.h"
#include "http.h"
#include "emulated/api68/api68.h"
#include "id3.h"
#include "soundTGDS.h"
#define ARM9COPY 0
#define ARM7COPY 1

#define STATE_PLAYING 0
#define STATE_PAUSED 1
#define STATE_STOPPED 2
#define STATE_UNLOADED 3

#define MP3_READ_SIZE (2048)
#define MP3_WRITE_SIZE (2048)
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

#define VGM_FREQ 32000
#define VGM_OUT_SIZE 2048

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


// mikmod
#include "mikmod/drivers/drv_nos.h"

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
extern ID3V1_TYPE id3Data;

// playback
extern bool initSoundStreamUser(char * fName, char * ext);
extern bool loadSound(char *fName);
extern void loadSoundGeneric(u32 wPlugin, int rate, int multiplicity, int sampleLength);
extern void pauseSound(bool pause);
extern void getSoundLoc(u32 *loc, u32 *max);
extern void setSoundLoc(u32 loc);
extern void closeSoundUser();
extern int getState();
extern int getSoundLength();
extern int getSourceFmt();
extern void soundPrevTrack();
extern void soundNextTrack();
extern void soundSetTrackInPayload(int trackIndex);
extern int getSNDHTrack();
extern int getSNDHTotalTracks();
extern void getSNDHMeta(api68_music_info_t * info);
extern int getSIDTrack();
extern int getSIDTotalTracks();
extern int getGBSTrack();
extern int getGBSTotalTracks();
extern char *gbsMeta(int which);

extern int getNSFTrack();
extern int getNSFTotalTracks();
extern char *getNSFMeta(int which);

// wifi
extern int getCurrentStatus();
//extern ICY_HEADER *getStreamData();

// for streaming record interrupt
extern void copyChunk();
extern int getStreamLag();
extern int getStreamLead();
//extern char *sidMeta(int which);

// for about screen to set or clear the ability to loop modules
extern void setLoop();
extern void clearLoop();
extern void checkEndSound();

extern u32 getSoundChannels();
extern int internalCodecType;
extern void copyData();

//VGM 
//extern int getVGMTrack();
extern void vgmDecode();
//extern int getVGMTotalTracks();

#ifdef __cplusplus
}
#endif
