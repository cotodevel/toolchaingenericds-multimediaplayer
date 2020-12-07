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
#include "sound.h"
#include "id3.h"
#include "ipcfifoTGDSUser.h"
#include "videoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "nds_cp15_misc.h"
#include "mad.h"
#include "flac.h"
#include "aacdec.h"
#include "main.h"

//Handles current file playback status
bool soundLoaded = false;

ID3V1_TYPE id3Data;
	
static bool canSend = false;
static int bufCursor;
static int bytesLeft = 0;
s16 *bytesLeftBuf = NULL;
static int maxBytes = 0;

bool cutOff = false;
bool sndPaused = false;
bool playing = false;
bool seekSpecial = false;
bool updateRequested = false;
int sndLen = 0;
int seekUpdate = -1;
extern ID3V1_TYPE id3Data;

// sound out
s16 *lBuffer = NULL;
s16 *rBuffer = NULL;

// wav
static bool memoryLoad = false;
static char *memoryContents = NULL;
static u32 memoryPos = 0;
static u32 memorySize = 0;
	
// mikmod
MODULE *module = NULL;
bool madFinished = false;
int sCursor = 0;
bool allowEQ = true;

// mp3
struct mad_stream Stream;
struct mad_frame Frame;
struct mad_synth Synth;
mad_timer_t Timer;
unsigned char *mp3Buf = NULL;

// ogg
OggVorbis_File vf;
int current_section;

// streaming
URL_TYPE curSite;
char *tmpMeta = NULL;
int streamMode = 0;
int s_socket;
int s_cursor;
int lagCursor;
char *s_buffer = NULL;
int s_retries = 0;
int s_count = 0;
int s_metaCount = 0;
int icyCopy = 0;
size_t oggStreamLoc = 0;
ICY_HEADER curIcy;
bool streamOpened = false;
static int tmpAmount = 0;
static int recAmount = 0;

// aac
HAACDecoder *hAACDecoder;
unsigned char *aacReadBuf = NULL;
unsigned char *aacReadPtr = NULL;
s16 *aacOutBuf = NULL;
AACFrameInfo aacFrameInfo;
int aacBytesLeft, aacRead, aacErr, aacEofReached;
int aacLength;
bool isRawAAC;
mp4ff_t *mp4file;
mp4ff_callback_t mp4cb;
int mp4track;
int sampleId;

//flac
FLACContext fc;
uint8_t *flacInBuf = NULL;
int flacBLeft = 0;
int32_t *decoded0 = NULL;
int32_t *decoded1 = NULL;
bool flacFinished = false;

//sid
char *sidfile = NULL;
static u32 sidLength = 0;
static unsigned short sid_load_addr, sid_init_addr, sid_play_addr;
static unsigned char sid_subSongsMax, sid_subSong, sid_song_speed;
static int nSamplesRendered, nSamplesPerCall, nSamplesToRender;

//nsf
uint8_t *nsffile = NULL;
static u32 nsfLength = 0;
static volatile bool inTrack = false;
bool isSwitching = false;

//spc
uint8_t *spcfile = NULL;
static u32 spcLength = 0;

//sndh
uint8_t *sndhfile = NULL;
static u32 sndhLength = 0;
static api68_init_t init68;
api68_t * sc68 = 0;
static int sndhTracks = 0;

//gbs
Music_Emu* emu;
static track_info_t info;
static int gbsTrack;
static int gbsOldTrack;

// arm7 code, etc

void SendArm7Command(u32 command, u32 data);
void copyRemainingData();
void fillMadBuffer();
void fillMadBufferStream();
void decodeMadBuffer(int mode);
void decodeMadBufferStream(int mode);
void startStream();
void recieveStream(int amount);

void aacFillBuffer();
size_t callbacks_read_func_stream(void *ptr, size_t size, size_t nmemb, void *datasource);
void decodeFlacFrame();
void seekFlac();
void sidDecode();
void nsfDecode();
void spcDecode();
void sndhDecode();
void gbsDecode();
void mp3Decode();

int getSIDTrack();
int getSIDTotalTracks();
int getGBSTrack();
int getGBSTotalTracks();
char *gbsMeta(int which);

// alternate malloc stuff
int m_SIWRAM = 0;
int m_size = 0;

void loadWavToMemory()
{
	memoryLoad = true;
}

void mallocData(int size)
{
	struct soundPlayerContext * soundPlayerCtx = &TGDSIPC->sndPlayerCtx;
	m_SIWRAM = 0;
	REG_SIWRAMCNT = 0; // arm9 owns both
	
	u32 *siTemp = (u32 *)SIWRAM0;
	int i=0;
	for(i=0;i<0x2000;++i)
		siTemp[i] = 0;
	
	m_size = size;
	
	lBuffer = SIWRAM0;
	rBuffer = SIWRAM0 + m_size;
	
	soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data = NULL; // temporary
	soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data = NULL; // temporary
	
	soundPlayerCtx->interlaced = NULL;
}

void freeData()
{
	lBuffer = NULL;
	rBuffer = NULL;
}

__attribute__((section(".itcm")))
void swapData()
{
	struct soundPlayerContext * soundPlayerCtx = &TGDSIPC->sndPlayerCtx;
	
	m_SIWRAM = 1 - m_SIWRAM;
	
	switch(m_SIWRAM)
	{
		case 0:
			lBuffer = SIWRAM0;
			rBuffer = SIWRAM0 + m_size;
			
			REG_SIWRAMCNT = 2; // bank 0 to arm9, bank 1 to arm7
			
			soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data = SIWRAM1;
			soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data = SIWRAM1 + m_size;
			
			soundPlayerCtx->interlaced = SIWRAM1;
			break;
		case 1:
			lBuffer = SIWRAM1;
			rBuffer = SIWRAM1 + m_size;
			
			REG_SIWRAMCNT = 1; // bank 0 to arm7, bank 1 to arm9
			
			soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_L_BUFFER].arm9data = SIWRAM0;
			soundPlayerCtx->soundSampleCxt[SOUNDSTREAM_R_BUFFER].arm9data = SIWRAM0 + m_size;
			
			soundPlayerCtx->interlaced = SIWRAM0;
			
			break;
	}
}

void parseSongTitle()
{
	char *tmpLower = (char *)trackMalloc(strlen(tmpMeta) + 1, "tmp buffer for lowercase");
	strcpy(tmpLower, tmpMeta);
	strlwr(tmpLower);
	
	char *tmpLoc = strstr(tmpLower, "streamtitle");
	
	if(tmpLoc)
	{
		int sizeIn = (tmpLoc - tmpLower) + 11;
		int foundOpen = 0;
		
		memset(curIcy.icyCurSong, 0, 1024);
		
		while(tmpMeta[sizeIn])
		{
			if(tmpMeta[sizeIn] == '\'')
			{
				if(foundOpen)
				{
					if(tmpMeta[sizeIn+1] == ';')
						break;
					else // still valid data
						curIcy.icyCurSong[strlen(curIcy.icyCurSong)] = tmpMeta[sizeIn];
				}
				else
					foundOpen = 1;
			}
			else
			{
				if(foundOpen)
					curIcy.icyCurSong[strlen(curIcy.icyCurSong)] = tmpMeta[sizeIn];
			}
			
			sizeIn++;
		}
	}
	
	trackFree(tmpLower);
}

__attribute__((section(".itcm")))
void circularCopy(void *dst, void *src, int copySize, int *position, int bufferSize)
{
	int count = 0;	
	
	while(count < copySize)
	{
		if(curIcy.icyMetaInt && icyCopy)
		{
			tmpMeta[strlen(tmpMeta)] = ((char *)src)[count];
			count++;
			icyCopy--;
			
			if(!icyCopy)
				parseSongTitle();
		}
		else if(curIcy.icyMetaInt && (s_metaCount == curIcy.icyMetaInt))
		{
			// metadata!
			int metaSize = ((char *)src)[count];
			metaSize *= 16;
			
			s_metaCount = 0;
			icyCopy = 0;
			count++;
			
			if(metaSize > 0)
			{
				icyCopy = metaSize;
				memset(tmpMeta, 0, 4081);
			}
		}
		else
		{
			((char *)dst)[*position] = ((char *)src)[count];
			
			count++;
			(*position)++;
			s_metaCount++;
			
			if(*position == bufferSize)
				*position = 0;
		}
	}
}

__attribute__((section(".itcm")))
void swapAndSend(u32 type)
{
	swapData();
	SendArm7Command(type,0);
}

//----------------------
// sound streaming stuff
//----------------------

__attribute__((section(".itcm")))
void SendArm7Command(u32 command, u32 data){
	SendFIFOWords((uint32)command, (uint32)data);
}

void setSoundInterrupt()
{
	//irqSet(IRQ_FIFO_NOT_EMPTY, FIFO_Receive);
	//irqEnable(IRQ_FIFO_NOT_EMPTY);

	//REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR | IPC_FIFO_RECV_IRQ;
}

int getSoundLength()
{
	return sndLen;
}

void freeStreamBuffer(){	
	if(s_buffer != NULL){
		trackFree(s_buffer);
		s_buffer = NULL;
	}
}

void allocateStreamBuffer()
{
	freeStreamBuffer();
	
	s_buffer = (char *)trackMalloc(STREAM_BUFFER_SIZE + 100, "circular buffer");
	memset(s_buffer, 0, STREAM_BUFFER_SIZE);
}

//----------------------
// sound streaming stuff
//----------------------

//---------------------
// sound stream wrapper
//---------------------

__attribute__((section(".itcm")))
void freeSound()
{
	stopSound(TGDSIPC->sndPlayerCtx.sourceFmt); //ARM9
	freeData();
}

void yield()
{	
	// dummy flac function
}

__attribute__((section(".itcm")))
void decodeFlacFrame()
{
	while(!flacFinished && !cutOff)
	{
		if(flac_decode_frame(&fc,decoded0,decoded1,flacInBuf,flacBLeft,yield) < 0) 
		{
			cutOff = true;
			return;
        }
		
		int consumed = fc.gb.index >> 3; // divide by 8
		
		memmove(flacInBuf,&flacInBuf[consumed],flacBLeft-consumed);
        flacBLeft-=consumed;
		
		// actual copy
		int i=0;
		for (i=0;i<fc.blocksize && !flacFinished;++i) 
		{
			lBuffer[bufCursor] = decoded0[i]; // left sample
			
			if(fc.channels == 2)
				rBuffer[bufCursor] = decoded1[i]; // right sample
			else
				rBuffer[bufCursor] = decoded0[i]; // left mono sample
			
			++bufCursor;
			
			if(bufCursor == FLAC_OUT_SIZE) // we've hit a filled buffer point
			{
				flacFinished = true;				
				bufCursor = 0;
				
				TGDSIPC->sndPlayerCtx.channels = fc.channels;
				maxBytes = FLAC_OUT_SIZE;
				
				++i;
				if(i < fc.blocksize)
				{	
					bytesLeft = fc.blocksize - i;
					
					if(fc.channels == 1)
					{
						bytesLeftBuf = (s16 *)trackMalloc(bytesLeft * 2, "temp flac holder");				
						s16 *tBuf = bytesLeftBuf;
						int j=0;
						for(j=0;j<bytesLeft;++j)
							*tBuf++ = decoded0[i+j];
					}
					else
					{
						bytesLeftBuf = (s16 *)trackMalloc(bytesLeft * 4, "temp flac holder");				
						s16 *tBuf = bytesLeftBuf;
						int j=0;
						for(j=0;j<bytesLeft;++j)
						{
							*tBuf++ = decoded0[i+j];
							*tBuf++ = decoded1[i+j];
						}
					}
				}
			}
        }
		
		// end copy
		
        int n = fread(&flacInBuf[flacBLeft], 1, MAX_FRAMESIZE-flacBLeft, TGDSIPC->sndPlayerCtx.filePointer);
        if(n) 
            flacBLeft+=n;
		else
			cutOff = true;
	}
}

__attribute__((section(".itcm")))
void seekFlac()
{
	while(1)
	{
		memset(flacInBuf, 0, MAX_FRAMESIZE);
		flacBLeft = fread(flacInBuf, 1, MAX_FRAMESIZE, TGDSIPC->sndPlayerCtx.filePointer);
		
		if(flacBLeft == 0)
		{
			cutOff = true;
			return;
		}	
		int i=0;
		for(i=0;i<flacBLeft-1;++i)
		{
			if(flacInBuf[i] == 0xFF)
			{
				if(flacInBuf[i+1] == 0xF8)
				{
					// found it!
					if(i)
					{
						memmove(flacInBuf, &flacInBuf[i], flacBLeft-i);
						memset(&flacInBuf[flacBLeft-i], 0, i);
						
						flacBLeft -= i;
					}
					
					return;
				}
			}
		}
		
		TGDSIPC->sndPlayerCtx.loc += flacBLeft;
	}
}

void sidDecode()
{
	s16 *tBuffer = lBuffer;
	
	nSamplesRendered = 0;
	while (nSamplesRendered < SID_OUT_SIZE)
	{
		if (nSamplesToRender == 0)
		{
			cpuJSR(sid_play_addr, 0);
			
			// Find out if cia timing is used and how many samples
			// have to be calculated for each cpujsr
			int nRefreshCIA = sidGetCIA();	
			if ((nRefreshCIA==0) || (sid_song_speed == 0)) 
				nRefreshCIA = 20000;
			nSamplesPerCall = sidGetMixFreq()*nRefreshCIA/1000000;
			
			nSamplesToRender = nSamplesPerCall;
		}
		if (nSamplesRendered + nSamplesToRender > SID_OUT_SIZE)
		{
			synth_render(tBuffer+nSamplesRendered, SID_OUT_SIZE-nSamplesRendered);
			nSamplesToRender -= SID_OUT_SIZE-nSamplesRendered;
			nSamplesRendered = SID_OUT_SIZE;
		}
		else
		{
			synth_render(tBuffer+nSamplesRendered, nSamplesToRender);
			nSamplesRendered += nSamplesToRender;
			nSamplesToRender = 0;
		} 
	}
}

char *sidMeta(int which)
{
	return sidfile + SID_META_LOC + (which << 5);
}

void mp3Decode()
{	
	madFinished = false;
	
	copyRemainingData();
	
	while(!madFinished)
	{	
		fillMadBuffer();
		decodeMadBuffer(1);
		//checkKeys();
	}
}

void nsfDecode()
{
	if(inTrack || isSwitching)
		return;
	
	inTrack = true;
	
	uint8_t *tBuffer = (uint8_t *)lBuffer;
	
    GetSamples(&tBuffer[0],NSF_OUT_SIZE/4);
	//checkKeys();
    GetSamples(&tBuffer[NSF_OUT_SIZE/4],NSF_OUT_SIZE/4);
	//checkKeys();
	GetSamples(&tBuffer[NSF_OUT_SIZE/2],NSF_OUT_SIZE/4);
	//checkKeys();
    GetSamples(&tBuffer[3*NSF_OUT_SIZE/4],NSF_OUT_SIZE/4);
	
	inTrack = false;
}

void spcDecode()
{
	spcPlay(lBuffer, rBuffer);
	//checkKeys();
	spcPlay(lBuffer + (SPC_OUT_SIZE >> 2), rBuffer + (SPC_OUT_SIZE >> 2));
	//checkKeys();
	spcPlay(lBuffer + (SPC_OUT_SIZE >> 1), rBuffer + (SPC_OUT_SIZE >> 1));
	//checkKeys();
	spcPlay(lBuffer + ((SPC_OUT_SIZE >> 2) * 3), rBuffer + ((SPC_OUT_SIZE >> 2) * 3));
}

void sndhDecode()
{
	int code = api68_process(sc68, lBuffer, SNDH_OUT_SIZE);
	
	if(code & API68_END) 
	{
		cutOff = true;
	}
}

void gbsDecode()
{
	gme_play(emu, GBS_OUT_SIZE, lBuffer);
	//checkKeys();
	gme_play(emu, GBS_OUT_SIZE, lBuffer + GBS_OUT_SIZE);
	
	// check for change track
	if(gbsTrack != gbsOldTrack)
	{
		//checkKeys();
		gme_start_track(emu, gbsTrack);
		gme_track_info(emu, &info, gbsTrack);
		
		gbsOldTrack = gbsTrack;
	}	
}

void *sndhMalloc(unsigned int t)
{
	return safeMalloc(t);
}

int getSNDHTrack()
{
	int curTrack = api68_play(sc68, -1, 0);
	
	return curTrack = 0 ? 1 : curTrack;
}

int getSNDHTotalTracks()
{
	return sndhTracks = 0 ? 1 : sndhTracks;
}

static inline u16 scale(mad_fixed_t sample)
{
	// round 
	sample += (1L << (MAD_F_FRACBITS - 16));

	// clip 
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	// quantize 
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

void fillMadBuffer()
{
	if(cutOff)
	{
		madFinished = true;
		return;
	}
	
	if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
	{
		size_t ReadSize, Remaining;
		unsigned char *ReadStart;
		
		if(Stream.next_frame!=NULL)
		{
			Remaining=Stream.bufend-Stream.next_frame;
			memmove(mp3Buf,Stream.next_frame,Remaining);
			ReadStart=(mp3Buf)+Remaining;
			ReadSize=MP3_READ_SIZE-Remaining;
		}
		else
		{
			ReadSize=MP3_READ_SIZE;
			ReadStart=mp3Buf;
			Remaining=0;
		}
		
		ReadSize = fread(ReadStart, 1, ReadSize, TGDSIPC->sndPlayerCtx.filePointer);
		
		if(ReadSize<=0)
		{
			madFinished = true;
			cutOff = true;
			return;
		}
		
		if(feof(TGDSIPC->sndPlayerCtx.filePointer))
		{
			unsigned char *GuardPtr=ReadStart+ReadSize;
			memset(GuardPtr,0,MAD_BUFFER_GUARD);
			ReadSize+=MAD_BUFFER_GUARD;
			madFinished = true;
			cutOff = true;
		}
		
		mad_stream_buffer(&Stream,mp3Buf,ReadSize+Remaining);
		Stream.error=MAD_ERROR_NONE;
	}
}

__attribute__((section(".itcm")))
void fillMadBufferStream()
{
	if(cutOff)
	{
		madFinished = true;
		return;
	}
	
	if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
	{
		size_t ReadSize, Remaining;
		unsigned char *ReadStart;
		
		if(Stream.next_frame!=NULL)
		{
			Remaining=Stream.bufend-Stream.next_frame;
			memmove(mp3Buf,Stream.next_frame,Remaining);
			ReadStart=(mp3Buf)+Remaining;
			ReadSize=STREAM_MP3_READ_SIZE-Remaining;
		}
		else
		{
			ReadSize=STREAM_MP3_READ_SIZE;
			ReadStart=mp3Buf;
			Remaining=0;
		}
		
		int maxReadSize = 0;
		if(lagCursor > s_cursor)
			maxReadSize = (s_cursor + STREAM_BUFFER_SIZE) - lagCursor;
		else
			maxReadSize = s_cursor - lagCursor;
		
		if((int)ReadSize > maxReadSize)
			ReadSize = maxReadSize;
		
		if(lagCursor == s_cursor)
		{
			unsigned char *GuardPtr=ReadStart+ReadSize;
			memset(GuardPtr,0,MAD_BUFFER_GUARD);
			ReadSize+=MAD_BUFFER_GUARD;
			madFinished = true;
			cutOff = true;
			
			mad_stream_buffer(&Stream,mp3Buf,ReadSize+Remaining);
			Stream.error=MAD_ERROR_NONE;
		}
		else
		{			
			if(lagCursor + ReadSize >= STREAM_BUFFER_SIZE) // weve looped around
			{
				int sizeToRead = STREAM_BUFFER_SIZE - lagCursor;
				int remaining = ReadSize - sizeToRead;
				
				memcpy(ReadStart, s_buffer + lagCursor, sizeToRead);
				memcpy(ReadStart + sizeToRead, s_buffer, remaining);			
				
				lagCursor += ReadSize;
				lagCursor -= STREAM_BUFFER_SIZE;
			}
			else
			{
				memcpy(ReadStart, s_buffer + lagCursor, ReadSize);
				lagCursor += ReadSize;
			}
			
			if(ReadSize<=0)
			{
				madFinished = true;
				cutOff = true;
				return;
			}		
			
			mad_stream_buffer(&Stream,mp3Buf,ReadSize+Remaining);
			Stream.error=MAD_ERROR_NONE;
		}
	}
}

__attribute__((section(".itcm")))
void decodeMadBuffer(int mode)
{	
	if(cutOff)
	{
		madFinished = true;
		return;
	}
	
	bool done = false;
	
	while(!done)
	{
		if(mad_frame_decode(&Frame,&Stream))
		{
			if(Stream.error!=MAD_ERROR_LOSTSYNC)
			{
				if(!MAD_RECOVERABLE(Stream.error))
				{
					if(Stream.error!=MAD_ERROR_BUFLEN)
					{
						cutOff = true;
						madFinished = true;
						
						return;
					}
					else
					{
						fillMadBuffer();				
					}
				}
				else
				{
					fillMadBuffer();	
				}
			}
		}
		else
		{
			done = true;
		}
		
		if(cutOff)
		{
			madFinished = true;
			return;
		}
	}
	
	mad_timer_add(&Timer,Frame.header.duration);
	if(mode == 1)
	{
		mad_synth_frame(&Synth,&Frame);
		copyData();
	}
}

__attribute__((section(".itcm")))
void decodeMadBufferStream(int mode)
{	
	if(cutOff)
	{
		madFinished = true;
		return;
	}
	
	bool done = false;
	
	while(!done)
	{
		if(mad_frame_decode(&Frame,&Stream))
		{
			if(!MAD_RECOVERABLE(Stream.error))
			{
				if(Stream.error!=MAD_ERROR_BUFLEN)
					return;
				else
					fillMadBufferStream();				
			}
			else
				fillMadBufferStream();	
		}
		else
			done = true;
		
		if(cutOff)
		{
			madFinished = true;
			return;
		}
	}

	mad_timer_add(&Timer,Frame.header.duration);
	if(mode == 1)
	{
		mad_synth_frame(&Synth,&Frame);
		
		copyData();
	}
}

__attribute__((section(".itcm")))
void copyData()
{
	if(Synth.pcm.length == 0)
		return;
	
	switch(TGDSIPC->sndPlayerCtx.channels)
	{
		case 1:{
			int lastPoint=0;
			for(lastPoint=0;lastPoint<Synth.pcm.length;lastPoint++)
			{
				u16 Sample;
				
				Sample=scale(Synth.pcm.samples[0][lastPoint]);		
				lBuffer[bufCursor] = Sample;
				rBuffer[bufCursor] = Sample;
				
				bufCursor++;
				
				if(bufCursor == MP3_WRITE_SIZE) // we hit the end of our double buffer
				{
					madFinished = true;
					
					lastPoint++;
					bufCursor = 0;
					maxBytes = MP3_WRITE_SIZE;
					
					if(lastPoint < Synth.pcm.length)
					{	
						bytesLeft = Synth.pcm.length - lastPoint;
						
						bytesLeftBuf = (s16 *)trackMalloc(bytesLeft * 2, "temp mp3 holder");				
						s16 *tBuf = bytesLeftBuf;
						int i=0;
						for(i=0;i<bytesLeft;++i)
							*tBuf++ = scale(Synth.pcm.samples[0][lastPoint + i]);
					}
					
					return;
				}
			}
		}
			break;
		case 2:{
			int lastPoint=0;
			for(lastPoint=0;lastPoint<Synth.pcm.length;lastPoint++)
			{
				u16 Sample;
				
				Sample=scale(Synth.pcm.samples[0][lastPoint]);
				lBuffer[bufCursor] = Sample;
				
				Sample=scale(Synth.pcm.samples[1][lastPoint]);
				rBuffer[bufCursor] = Sample;
				
				bufCursor++;
				
				if(bufCursor == MP3_WRITE_SIZE) // we hit the end of our double buffer
				{
					madFinished = true;
					
					lastPoint++;
					bufCursor = 0;
					maxBytes = MP3_WRITE_SIZE;
					
					if(lastPoint < Synth.pcm.length)
					{	
						bytesLeft = Synth.pcm.length - lastPoint;
						
						bytesLeftBuf = (s16 *)trackMalloc(bytesLeft * 4, "temp mp3 holder");				
						s16 *tBuf = bytesLeftBuf;
						int i=0;
						for(i=0;i<bytesLeft;++i)
						{
							*tBuf++ = scale(Synth.pcm.samples[0][lastPoint + i]);
							*tBuf++ = scale(Synth.pcm.samples[1][lastPoint + i]);
						}
					}
					
					return;
				}
			}
		}
			break;
	}
	
	return;
}


__attribute__((section(".itcm")))
void copyRemainingData()
{
	bufCursor = 0;
	
	if(!bytesLeft)
		return;
	
	s16 *tBuf = bytesLeftBuf;
	
	int max = bytesLeft;
	
	if(max >= maxBytes)
	{
		bytesLeft -= maxBytes;		
		max = maxBytes;
		
		madFinished = true;
		flacFinished = true;
	}
	else{
		bytesLeft = 0;
	}
	
	switch(TGDSIPC->sndPlayerCtx.channels)
	{
		case 1:{
			int i=0;
			for(i=0;i<max;i++)
			{
				u16 Sample;
				
				Sample = *tBuf++;
				lBuffer[bufCursor] = Sample;
				rBuffer[bufCursor] = Sample;
				
				bufCursor++;
			}
		}
			break;
		case 2:{
			int i=0;
			for(i=0;i<max;i++)
			{
				u16 Sample;
				
				Sample = *tBuf++;
				lBuffer[bufCursor] = Sample;
				
				Sample = *tBuf++;				
				rBuffer[bufCursor] = Sample;
				
				bufCursor++;
			}
		}
			break;
	}
	
	if(bytesLeft == 0)
	{
		if(bytesLeftBuf)
		{
			trackFree(bytesLeftBuf);
			bytesLeftBuf = NULL;
		}
	}
}


__attribute__((section(".itcm")))
static int FillReadBuffer(unsigned char *readBuf, unsigned char *readPtr, int bufSize, int bytesLeft)
{
	int nRead;

	/* move last, small chunk from end of buffer to start, then fill with new data */
	if(bytesLeft > 0)
		memmove(readBuf, readPtr, bytesLeft);
	
	if(isRawAAC) // raw aac
	{
		if(TGDSIPC->sndPlayerCtx.sourceFmt == SRC_AAC){
			nRead = fread(readBuf + bytesLeft, 1, bufSize - bytesLeft, TGDSIPC->sndPlayerCtx.filePointer);		
		}
		else // SRC_STREAM_AAC
		{
			size_t ReadSize = bufSize - bytesLeft;
			int maxReadSize = 0;
			
			if(lagCursor > s_cursor){
				maxReadSize = (s_cursor + STREAM_BUFFER_SIZE) - lagCursor;
			}
			else{
				maxReadSize = s_cursor - lagCursor;
			}
			
			if((int)ReadSize > maxReadSize){
				ReadSize = maxReadSize;
			}
			
			if(lagCursor == s_cursor){
				nRead = 0;
			}
			else
			{			
				if(lagCursor + ReadSize >= STREAM_BUFFER_SIZE) // weve looped around
				{
					int sizeToRead = STREAM_BUFFER_SIZE - lagCursor;
					int remaining = ReadSize - sizeToRead;
					
					memcpy(readBuf + bytesLeft, s_buffer + lagCursor, sizeToRead);
					memcpy(readBuf + bytesLeft + sizeToRead, s_buffer, remaining);			
					
					lagCursor += ReadSize;
					lagCursor -= STREAM_BUFFER_SIZE;
				}
				else
				{
					memcpy(readBuf + bytesLeft, s_buffer + lagCursor, ReadSize);
					lagCursor += ReadSize;
				}
				
				nRead = ReadSize;
			}
		}
	}
	else
	{
		// read next frame from m4a/m4b file
		if(sampleId < (int)TGDSIPC->sndPlayerCtx.len)
		{
			nRead = mp4ff_read_sample_v2(mp4file, mp4track, sampleId++, (unsigned char *)(readBuf + bytesLeft));
		}
		else
		{
			nRead = 0;
		}
	}

	/* zero-pad to avoid finding false sync word after last frame (from old data in readBuf) */
	if (nRead < bufSize - bytesLeft)
	{
		memset(readBuf + bytesLeft + nRead, 0, bufSize - bytesLeft - nRead);	
	}

	return nRead;
}

__attribute__((section(".itcm")))
void aacFillBuffer()
{
	if (aacBytesLeft < AAC_MAX_NCHANS * AAC_MAINBUF_SIZE && !aacEofReached) 
	{
		aacRead = FillReadBuffer(aacReadBuf, aacReadPtr, AAC_READBUF_SIZE, aacBytesLeft);
		aacBytesLeft += aacRead;
		aacReadPtr = aacReadBuf;
		if (aacRead == 0)
		{
			aacEofReached = 1;
		}
	}
}

__attribute__((section(".itcm")))
int find_first_aac_track(mp4ff_t *infile)
{
	int i, trackType;
	int numTracks = mp4ff_total_tracks(infile);

	for (i = 0; i < numTracks; i++)
	{
		trackType = mp4ff_get_track_type(infile, 0);
				
		if (trackType < 0)
			continue;
		if (trackType >= 1 && trackType <= 4) // first aac track
			return i;
	}

	// no aac track
	return -1;
}

// aac normal callbacks

uint32_t aac_read_callback(void *user_data, void *buffer, uint32_t length)
{
	return fread(buffer, 1, length, (FILE*)user_data);
}

uint32_t aac_seek_callback(void *user_data, uint64_t position)
{
	return fseek((FILE*)user_data, position, SEEK_SET);
}

// callbacks for ogg vorbis, craftily snuck from moonshell source because
// I was far too lazy to write them myself....yet I ended up modifying them
// all ....

size_t callbacks_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE *)datasource);
}

int callbacks_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	return fseek((FILE *)datasource, offset, whence);
}

int callbacks_close_func(void *datasource)
{
	fclose((FILE*)datasource);
	
	return 0;
}

long callbacks_tell_func(void *datasource)
{
	return(ftell((FILE *)datasource));
}

// callbacks for ogg vorbis streaming
// only read is valid

size_t callbacks_read_func_stream(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	size_t ReadSize = size * nmemb;
	int maxReadSize = 0;
	
	if(lagCursor > s_cursor)
		maxReadSize = (s_cursor + STREAM_BUFFER_SIZE) - lagCursor;
	else
		maxReadSize = s_cursor - lagCursor;
	
	if((int)ReadSize > maxReadSize)
		ReadSize = maxReadSize;
		
	if(lagCursor == s_cursor)
		return 0;
	else
	{			
		if(lagCursor + ReadSize >= STREAM_BUFFER_SIZE) // weve looped around
		{
			int sizeToRead = STREAM_BUFFER_SIZE - lagCursor;
			int remaining = ReadSize - sizeToRead;
			
			memcpy(ptr, s_buffer + lagCursor, sizeToRead);
			memcpy((char *)ptr + sizeToRead, s_buffer, remaining);			
			
			lagCursor += ReadSize;
			lagCursor -= STREAM_BUFFER_SIZE;
		}
		else
		{
			memcpy(ptr, s_buffer + lagCursor, ReadSize);
			lagCursor += ReadSize;
		}
	}
	
	oggStreamLoc += ReadSize;
	return ReadSize / size;
}

int callbacks_seek_func_stream(void *datasource, ogg_int64_t offset, int whence)
{
	switch(whence)
	{
		case SEEK_SET:{
			int addLoc = offset - oggStreamLoc;
			
			oggStreamLoc += addLoc;
			lagCursor += addLoc;
		}
		break;
		case SEEK_CUR:{
			oggStreamLoc += offset;
			lagCursor += offset;	
		}
		break;
		case SEEK_END:{
			// unsupported
			return -1;
		}
		break;
	}
	
	if(lagCursor >= STREAM_BUFFER_SIZE)
		lagCursor -= STREAM_BUFFER_SIZE;
	if(lagCursor < 0)
		lagCursor += STREAM_BUFFER_SIZE;

	return 0;
}

int callbacks_close_func_stream(void *datasource)
{
	return 0;
}

long callbacks_tell_func_stream(void *datasource)
{
	return oggStreamLoc;
}

// main sound init routine

void setLoop()
{
	module->wrap = 1;
}

void clearLoop()
{
	module->wrap = 0;
}

bool loadSound(char *fName)
{	
	char tmpName[256];
	char ext[256];
	
	strcpy(tmpName, fName);	
	separateExtension(tmpName, ext);
	strlwr(ext);
	
	freeSound();	
	lBuffer = NULL;
	rBuffer = NULL;
	
	cutOff = false;
	sndPaused = false;
	playing = false;
	seekUpdate = -1;
	
	// try this first to prevent false positives with other streams
	
	/*
	if(isURL(fName))
	{	
		//it's a url
		if(!urlFromString(fName, &curSite))
		{
			return false;
		}
		
		strcpy(tmpName, curSite.remotePath);
		separateExtension(tmpName, ext);
		strlwr(ext);
		
		if(strcmp(ext, ".ogg") == 0)
			TGDSIPC->sndPlayerCtx.sourceFmt = SRC_STREAM_OGG;
		else if(strcmp(ext, ".aac") == 0)
			TGDSIPC->sndPlayerCtx.sourceFmt = SRC_STREAM_AAC;		
		else
			TGDSIPC->sndPlayerCtx.sourceFmt = SRC_STREAM_MP3;
		
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		if(isWIFIConnected())
		{
			disconnectWifi();
		}
		
		streamMode = STREAM_DISCONNECTED;
		streamOpened = false;
		mp3Buf = NULL;
		
		freeStreamBuffer();
		
		tmpMeta = (char *)trackMalloc(4081, "metadata");
		memset(&curIcy, 0, sizeof(ICY_HEADER));	
		memset(tmpMeta, 0, 4081);
		
		return true;
	}
	*/

	if(strcmp(ext,".wav") == 0)
	{
		// wav file!
		return initSoundStream(fName);
	}
	
	DisableSoundSampleContext();	//Disable ARM7 TGDS Sound stuff because decoders require a lot of power
	if(strcmp(ext, ".it") == 0  || strcmp(ext, ".mod") == 0 || strcmp(ext, ".s3m") == 0 || strcmp(ext, ".xm") == 0)
	{
		// tracker file!
		
		md_mode = 0;
		md_mode |= DMODE_16BITS;
		md_mode |= DMODE_SOFT_MUSIC;
		md_mode |= DMODE_SOFT_SNDFX;
		md_mode |= DMODE_STEREO;
		
		md_mixfreq = MIKMOD_FREQ;
		
		MikMod_Init("");
		
		struct soundPlayerContext * soundPlayerCtx = &TGDSIPC->sndPlayerCtx;
		soundPlayerCtx->channels = 2; // for de-interlacing
		
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_MIKMOD;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		module = Player_Load(fName, 256, 0);
		
		if(module == NULL)
			return false;
		
		Player_Start(module);
		
		setSoundInterpolation(1);
		setSoundFrequency(MIKMOD_FREQ);
		setSoundLengthDSO(ZEROLEN >> 2);
		
		mallocData(ZEROLEN >> 2);
		
		TGDSIPC->sndPlayerCtx.len = module->numpos;
		TGDSIPC->sndPlayerCtx.loc = 0;
		sCursor = 0;
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		
		return true;		
	}
	
	if(strcmp(ext, ".mp3") == 0 || strcmp(ext, ".mp2") == 0 || strcmp(ext, ".mpa") == 0)
	{
		// mp3 file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_MP3;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		mad_stream_init(&Stream);
		mad_frame_init(&Frame);
		mad_synth_init(&Synth);
		mad_timer_reset(&Timer);
		
		FILE *fp = fopen(fName, "r");
		TGDSIPC->sndPlayerCtx.filePointer = fp;
		
		getID3V1(fp, &id3Data);
		
		mp3Buf = (unsigned char *)trackMalloc(MP3_READ_SIZE,"mp3 buffer");		
		mallocData(MP3_WRITE_SIZE);		
		TGDSIPC->sndPlayerCtx.mp3SampleRate = 0;
		
		int retries = 0;
		
		while(TGDSIPC->sndPlayerCtx.mp3SampleRate == 0 || Stream.error != MAD_ERROR_NONE)
		{	
			fillMadBuffer();
			decodeMadBuffer(0);
			
			TGDSIPC->sndPlayerCtx.mp3SampleRate = Frame.header.samplerate;
			
			retries++;
			
			if(retries > 100)
			{
				mad_synth_finish(&Synth);
				mad_frame_finish(&Frame);
				mad_stream_finish(&Stream);
				
				fclose(fp);
				
				return false;
			}
		}
		
		TGDSIPC->sndPlayerCtx.channels = MAD_NCHANNELS(&Frame.header);
		
		mad_synth_finish(&Synth);
		mad_frame_finish(&Frame);
		mad_stream_finish(&Stream);
		
		fseek(fp, 0, SEEK_SET);
		
		mad_stream_init(&Stream);
		mad_frame_init(&Frame);
		mad_synth_init(&Synth);
		mad_timer_reset(&Timer);
		
		setSoundLengthDSO(MP3_WRITE_SIZE);
		
		if(strcmp(ext, ".mp3") != 0)
		{
			setSoundInterpolation(2);
		}
		else
		{
			setSoundInterpolation(4);
		}
		
		setSoundFrequency(TGDSIPC->sndPlayerCtx.mp3SampleRate);
		
		TGDSIPC->sndPlayerCtx.len = flength(fp) - 1000;
		TGDSIPC->sndPlayerCtx.loc = 0;
		bufCursor = 0;
		bytesLeft = 0;
		bytesLeftBuf = NULL;
		
		mp3Decode();	//process header frame
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		return true;
	}
	
	if(strcmp(ext, ".ogg") == 0)
	{
		// ogg file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_OGG;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		FILE *fp = fopen(fName, "r");
		TGDSIPC->sndPlayerCtx.filePointer = fp;
		
		ov_callbacks oggCallBacks = {callbacks_read_func, callbacks_seek_func, callbacks_close_func, callbacks_tell_func};
		
		if(ov_open_callbacks(fp, &vf, NULL, 0, oggCallBacks) < 0)
		{
			fclose(fp);
			return false;
		}
		
		vorbis_info *vi = ov_info(&vf,-1);
		
		getOggInfo(&vf, &id3Data);
		
		TGDSIPC->sndPlayerCtx.channels = vi->channels;		
		if(TGDSIPC->sndPlayerCtx.channels > 2)
		{
			bool closeOGGFileHandleInternally = false;	//no, can't close the filehandle here through callbacks. Close it through the DRAGON_xxxx library
			ov_clear(&vf, closeOGGFileHandleInternally);
			fclose(fp);
			return false; // no more than 2 channels, sorry
		}
		
		setSoundInterpolation(1);
		setSoundFrequency(vi->rate);		
		setSoundLengthDSO(OGG_READ_SIZE);
		mallocData(OGG_READ_SIZE);		
		
		bufCursor = 0;
		
		TGDSIPC->sndPlayerCtx.len = flength(fp);
		TGDSIPC->sndPlayerCtx.loc = 0;
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		
		return true;
	}
	
	if(strcmp(ext, ".aac") == 0)
	{
		// raw aac file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_AAC;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		hAACDecoder = (HAACDecoder *)AACInitDecoder();
		if (!hAACDecoder)
			return false;
		
		FILE *fp = fopen(fName, "r");
		TGDSIPC->sndPlayerCtx.filePointer = fp;
		
		aacReadBuf = (unsigned char *)trackMalloc(AAC_READBUF_SIZE, "aac read stream");
		aacOutBuf = (s16 *)trackMalloc(AAC_OUT_SIZE, "aac out stream");
		aacReadPtr = aacReadBuf;
		aacBytesLeft = 0;
		aacEofReached = 0;
		aacErr = ERR_AAC_INDATA_UNDERFLOW;
		isRawAAC = true;
		
		while(aacErr == ERR_AAC_INDATA_UNDERFLOW)
		{
			aacFillBuffer();
			aacErr = AACDecode(hAACDecoder, &aacReadPtr, &aacBytesLeft, aacOutBuf);
		}
		AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);
		
		if(aacFrameInfo.bitsPerSample != 16) // sorry, unsupported
		{
			if(aacReadBuf)
				trackFree(aacReadBuf);			
			if(aacOutBuf)
				trackFree(aacOutBuf);
				
			aacReadBuf = NULL;
			aacOutBuf = NULL;
			
			fclose(fp);
			
			return false;
		}
		
		TGDSIPC->sndPlayerCtx.channels = aacFrameInfo.nChans;	
		if(TGDSIPC->sndPlayerCtx.channels > 2 || TGDSIPC->sndPlayerCtx.channels == 0)
		{
			if(aacReadBuf)
				trackFree(aacReadBuf);			
			if(aacOutBuf)
				trackFree(aacOutBuf);
				
			aacReadBuf = NULL;
			aacOutBuf = NULL;
			
			fclose(fp);
			
			return false;
		}
		
		setSoundInterpolation(2);
		setSoundFrequency(aacFrameInfo.sampRateOut);		
		setSoundLengthDSO(aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);
		mallocData(aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);	
		
		memcpy(lBuffer, aacOutBuf, aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);
		trackFree(aacOutBuf);		
		aacOutBuf = NULL;
		
		aacLength = aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels;
		
		TGDSIPC->sndPlayerCtx.len = flength(fp);
		TGDSIPC->sndPlayerCtx.loc = 0;
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		
		return true;
	}
	
	if(strcmp(ext, ".m4a") == 0 || strcmp(ext, ".m4b") == 0)
	{
		// raw apple wrapper around aac file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_AAC;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		hAACDecoder = (HAACDecoder *)AACInitDecoder();
		if (!hAACDecoder)
			return false;
			
		FILE *fp = fopen(fName, "r");
		TGDSIPC->sndPlayerCtx.filePointer = fp;
		
		aacReadBuf = (unsigned char *)trackMalloc(AAC_READBUF_SIZE * 2, "aac read stream");
		aacOutBuf = (s16 *)trackMalloc(AAC_OUT_SIZE, "aac out stream");
		aacReadPtr = aacReadBuf;
		aacBytesLeft = 0;
		aacEofReached = 0;
		aacErr = ERR_AAC_INDATA_UNDERFLOW;
		isRawAAC = false;
		
		// invoke shit with m4a decoding
		
		mp4cb.read = aac_read_callback;
		mp4cb.seek = aac_seek_callback;
		mp4cb.user_data = fp;		
		
		mp4file = NULL;
		mp4file = mp4ff_open_read(&mp4cb);
		if(!mp4file) 
		{
			mp4ff_close(mp4file);
			
			if(aacReadBuf)
				trackFree(aacReadBuf);			
			if(aacOutBuf)
				trackFree(aacOutBuf);
				
			aacReadBuf = NULL;
			aacOutBuf = NULL;
			
			return false;
		}
		
		if((mp4track = find_first_aac_track(mp4file)) < 0) 
		{
			mp4ff_close(mp4file);
			
			if(aacReadBuf)
				trackFree(aacReadBuf);			
			if(aacOutBuf)
				trackFree(aacOutBuf);
				
			aacReadBuf = NULL;
			aacOutBuf = NULL;
			
			return false;
		}
		
		unsigned char *aacbuf = NULL;
		unsigned int aacbuf_len;
		
		mp4ff_get_decoder_config(mp4file, mp4track, &aacbuf, &aacbuf_len);
		
		if(aacbuf_len == 2)
		{
			int sample_rates[] = {	96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000,  7350,  0,     0,     0 };
			
			int sampRateIndex = ((aacbuf[0] << 1) & 0xe) | ((aacbuf[1] >> 7) & 0x1);
			
			aacFrameInfo.sampRateCore = sample_rates[sampRateIndex];
			aacFrameInfo.nChans = ((aacbuf[1]) >> 3) & 0xf;
		}
		
		safeFree(aacbuf);
		
		TGDSIPC->sndPlayerCtx.len = mp4ff_num_samples(mp4file, mp4track);
		sampleId = 0;
		
		AACSetRawBlockParams(hAACDecoder, 0, &aacFrameInfo);
		
		// back to normal now
		
		while(aacErr == ERR_AAC_INDATA_UNDERFLOW)
		{
			aacFillBuffer();
			aacErr = AACDecode(hAACDecoder, &aacReadPtr, &aacBytesLeft, aacOutBuf);
		}
		AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);
		
		if(aacFrameInfo.bitsPerSample != 16) // sorry, unsupported
		{
			mp4ff_close(mp4file);
			
			if(aacReadBuf)
				trackFree(aacReadBuf);			
			if(aacOutBuf)
				trackFree(aacOutBuf);
				
			aacReadBuf = NULL;
			aacOutBuf = NULL;
			
			return false;
		}
		
		TGDSIPC->sndPlayerCtx.channels = aacFrameInfo.nChans;	
		if(TGDSIPC->sndPlayerCtx.channels > 2)
		{
			mp4ff_close(mp4file);
			
			if(aacReadBuf)
				trackFree(aacReadBuf);			
			if(aacOutBuf)
				trackFree(aacOutBuf);
				
			aacReadBuf = NULL;
			aacOutBuf = NULL;
			
			return false;
		}
		
		setSoundInterpolation(1);
		setSoundFrequency(aacFrameInfo.sampRateOut);		
		setSoundLengthDSO(aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);
		mallocData(aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);	
		
		memcpy(lBuffer, aacOutBuf, aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);		
		trackFree(aacOutBuf);
		aacOutBuf = NULL;
		
		aacLength = aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels;
		TGDSIPC->sndPlayerCtx.loc = 0;
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		
		return true;
	}
	
	if(strcmp(ext, ".flac") == 0)
	{
		// flac audio file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_FLAC;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		TGDSIPC->sndPlayerCtx.filePointer = fopen(fName,"r");
		if(!flac_init(TGDSIPC->sndPlayerCtx.filePointer,&fc))
		{
			fclose(TGDSIPC->sndPlayerCtx.filePointer);
			return false;			
		}
		
		if(fc.channels > 2) // sorry, only mono or stereo
		{
			fclose(TGDSIPC->sndPlayerCtx.filePointer);			
			return false;			
		}
		
		flacInBuf = (uint8_t *)trackMalloc(MAX_FRAMESIZE, "flac in buffer");
		flacBLeft = fread(flacInBuf, 1, MAX_FRAMESIZE, TGDSIPC->sndPlayerCtx.filePointer);
		
		decoded0 = (int32_t *)trackMalloc(MAX_BLOCKSIZE * sizeof(int32_t), "flac out buffer0");
		decoded1 = (int32_t *)trackMalloc(MAX_BLOCKSIZE * sizeof(int32_t), "flac out buffer1");
		
		setSoundInterpolation(1);
		setSoundFrequency(fc.samplerate);	
		setSoundLengthDSO(FLAC_OUT_SIZE);
		
		mallocData(FLAC_OUT_SIZE);
		
		bufCursor = 0;		
		TGDSIPC->sndPlayerCtx.len = fc.filesize;
		TGDSIPC->sndPlayerCtx.loc = 0;
		bytesLeft = 0;
		bytesLeftBuf = NULL;
    	
		decodeFlacFrame();
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		
		return true;
	}
	
	if(strcmp(ext, ".sid") == 0)
	{
		// sid audio file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_SID;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		FILE *df = fopen(fName,"r");
		sidLength = flength(df);
		
		sidfile = (char *)trackMalloc(sidLength, "sid file temporary");
		fread(sidfile, 1, sidLength, df);
		fclose(df);
		
		sid_load_addr = 0;
		sid_init_addr = 0;
		sid_play_addr = 0;
		sid_subSongsMax = 0;
		sid_subSong = 0;
		sid_song_speed = 0;
		
		nSamplesRendered = 0;
		nSamplesPerCall = 882;  // This is PAL SID single speed (44100/50Hz)
		nSamplesToRender = 0;
		
		c64Init(SID_FREQ);
		LoadSIDFromMemory(sidfile, &sid_load_addr, &sid_init_addr, &sid_play_addr, &sid_subSongsMax, &sid_subSong, &sid_song_speed, sidLength);
		sidPoke(24, 15);                // Turn on full volume
		cpuJSR(sid_init_addr, sid_subSong);     // Start the song initialize
		
		TGDSIPC->sndPlayerCtx.channels = 1;
		setSoundInterpolation(2);
		setSoundFrequency(SID_FREQ);	
		setSoundLengthDSO(SID_OUT_SIZE);
		
		mallocData(SID_OUT_SIZE);
		
		// make sure the meta data isn't fucked up or overflowing
		sidfile[SID_META_LOC + 31] = 0;
		sidfile[SID_META_LOC + 63] = 0;
		sidfile[SID_META_LOC + 95] = 0;
		
		sidDecode();
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);		
		
		return true;
	}	
	
	if(strcmp(ext, ".nsf") == 0)
	{
		// NES audio file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_NSF;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		FILE *df = fopen(fName,"r");
		nsfLength = flength(df);
		
		nsffile = (uint8_t *)trackMalloc(nsfLength, "nsf file temporary");
		fread(nsffile, 1, nsfLength, df);
		fclose(df);
		
		if(!NSFCore_Initialize())
		{
			trackFree(nsffile);
			return false;
		}
		
		RebuildOutputTables();
		
		if(LoadFile(nsffile,nsfLength))
		{
			trackFree(nsffile);			
			NSFCore_Free();
			return false;
		}
		
		if(!SetPlaybackOptions(NSF_FREQ))
		{
			trackFree(nsffile);			
			NSFCore_Free();
			return false;
		}
		
		if(!LoadNSF(getnDataBufferSize()))
		{
			trackFree(nsffile);			
			NSFCore_Free();
			return false;
		}
		
		SetTrack(0);
		inTrack = false;
		
		TGDSIPC->sndPlayerCtx.channels = 1;
		setSoundInterpolation(2);
		setSoundFrequency(NSF_FREQ);	
		setSoundLengthDSO(NSF_OUT_SIZE/2);
		
		mallocData(NSF_OUT_SIZE);
		
		StopFade();
		nsfDecode();
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);	
		
		return true;
	}
	
	if(strcmp(ext, ".spc") == 0)
	{
		// SNES audio file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_SPC;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		FILE *df = fopen(fName,"r");
		spcLength = flength(df);
		
		spcfile = (uint8_t *)trackMalloc(spcLength, "spc file temporary");
		fread(spcfile, 1, spcLength, df);
		fclose(df);
		
		if(!spcInit(spcfile, spcLength))
		{
			trackFree(spcfile);
			return false;
		}
		
		TGDSIPC->sndPlayerCtx.channels = 2;
		setSoundInterpolation(2);
		setSoundFrequency(SPC_FREQ);	
		setSoundLengthDSO(SPC_OUT_SIZE);
		
		mallocData(SPC_OUT_SIZE);
		
		spcDecode();
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		
		return true;
	}
	
	if(strcmp(ext, ".sndh") == 0 || strcmp(ext, ".snd") == 0 || strcmp(ext, ".sc68") == 0)
	{
		// Atari ST audio file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_SNDH;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		FILE *df = fopen(fName,"r");
		sndhLength = flength(df);
		
		sndhfile = (uint8_t *)trackMalloc(sndhLength, "sndh file temporary");
		fread(sndhfile, 1, sndhLength, df);
		fclose(df);
		
		// Clean up init structure (required).
		memset(&init68, 0, sizeof(init68));
		
		// Set dynamic handler (required).
		init68.alloc = sndhMalloc;
		init68.free = safeFree;
		
		sc68 = api68_init(&init68);
		
		if(!sc68)
		{
			trackFree(sndhfile);
			return false;
		}
		
		// Load an sc68 file.
		if(api68_load_mem(sc68, sndhfile, sndhLength) < 0) 
		{
			api68_shutdown(sc68);
			trackFree(sndhfile);
			return false;
		}
		
		sndhTracks = api68_tracks(sc68);
		
		// Set a track (optionnal).
		api68_play(sc68, 0, 0);
		
		TGDSIPC->sndPlayerCtx.channels = 1;
		setSoundInterpolation(1);
		setSoundFrequency(init68.sampling_rate);	
		setSoundLengthDSO(SNDH_OUT_SIZE);
		
		mallocData(SNDH_OUT_SIZE);
		
		sndhDecode();
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		
		return true;
	}
	
	if(strcmp(ext, ".gbs") == 0)
	{
		// GameBoy audio file
		
		TGDSIPC->sndPlayerCtx.sourceFmt = SRC_GBS;
		TGDSIPC->sndPlayerCtx.bufLoc = 0;
		
		FILE *df = fopen(fName,"r");
		int size = flength(df);
		
		char *data = (char *)safeMalloc(size);
		fread(data, 1, size, df);
		fclose(df);
		
		gme_open_data(data, size, &emu, GBS_FREQ);
		safeFree(data);
		
		gbsTrack = 0;
		gbsOldTrack = 0;
		
		gme_track_info(emu, &info, gbsTrack);
		
		gme_start_track(emu, gbsTrack);
		
		TGDSIPC->sndPlayerCtx.channels = 2;
		setSoundInterpolation(1);
		setSoundFrequency(GBS_FREQ);	
		setSoundLengthDSO(GBS_OUT_SIZE);		
		mallocData(GBS_OUT_SIZE);
		
		gbsDecode();
		startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
		
		return true;
	}
	
	return false;
}

void soundPrevTrack(int x, int y)
{
	switch(TGDSIPC->sndPlayerCtx.sourceFmt)
	{
		case SRC_NSF:			
		{
			if(getNSFTrack()-1 < 1)
				return;
			
			int tTrack = getNSFTrack()-2;
			
			while(inTrack);
			
			isSwitching = true;
			
			NSFCore_Initialize();
			RebuildOutputTables();
			LoadFile(nsffile,nsfLength);
			SetPlaybackOptions(NSF_FREQ);
			LoadNSF(getnDataBufferSize());
			SetTrack(tTrack);
			
			StopFade();
			
			isSwitching = false;
			
			nsfDecode();
		}
		break;
		case SRC_SNDH:
		{
			if(getSNDHTrack() - 1 < 1)
				return;
			
			api68_play(sc68, getSNDHTrack() - 1, 0);
		}
		break;
		case SRC_SID:
		{
			if(getSIDTrack() - 1 < 1)
				return;
			
			cpuJSR(sid_init_addr, --sid_subSong);     // Start the song initialize
		}
		break;
		case SRC_GBS:
		{
			if(getGBSTrack() - 1 < 1){
				return;
			}
			gbsTrack--;
		}
		break;
	}	
}

void soundNextTrack(int x, int y)
{
	switch(TGDSIPC->sndPlayerCtx.sourceFmt)
	{
		case SRC_NSF:			
		{
			if(getNSFTrack()+1 > getNSFTotalTracks())
				return;
			
			int tTrack = getNSFTrack();
			
			while(inTrack);
			
			isSwitching = true;
			
			NSFCore_Initialize();
			RebuildOutputTables();
			LoadFile(nsffile,nsfLength);
			SetPlaybackOptions(NSF_FREQ);
			LoadNSF(getnDataBufferSize());
			SetTrack(tTrack);
			
			StopFade();
			
			isSwitching = false;
			
			nsfDecode();
		}
		break;
		case SRC_SNDH:
		{
			if(getSNDHTrack()+1 > getSNDHTotalTracks())
				return;
			
			api68_play(sc68, getSNDHTrack() + 1, 0);
		}
		break;
		case SRC_SID:
		{
			if(getSIDTrack()+1 > getSIDTotalTracks())
				return;
			
			cpuJSR(sid_init_addr, ++sid_subSong);     // Start the song initialize
			
		}
		break;
		case SRC_GBS:
		{
			if(getGBSTrack()+1 > getGBSTotalTracks())
				return;
			
			gbsTrack++;
		}
		break;
	}
}

void getSNDHMeta(api68_music_info_t *info)
{
	api68_music_info(sc68, info, -1, 0);
}

void startStream()
{
	//if(TGDSIPC->sndPlayerCtx.sourceFmt == SRC_STREAM_MP3)
	//	s_socket = sendStreamRequest(curSite.serverIP, curSite.serverPort, curSite.remotePath, true); // true for metadata, false for none
	//else
		s_socket = sendStreamRequest(curSite.serverIP, curSite.serverPort, curSite.remotePath, false); // true for metadata, false for none
}

void checkStream() // check if we have recieved anything yet
{
	char *tcpTemp = (char *)safeMalloc(STREAM_WIFI_READ_SIZE);
	int i = recvData(s_socket,tcpTemp,STREAM_WIFI_READ_SIZE);
	
	if(i == 0) 
	{
		// finished unexpectedly
		streamMode = STREAM_FAILEDCONNECT;
		
		safeFree(tcpTemp);		
		return;
	}
	
	if(i > 0)
	{
		circularCopy(s_buffer, tcpTemp, i, &s_cursor, STREAM_BUFFER_SIZE);
		
		char *tmp = strstr(s_buffer, "\r\n");
		while(tmp != NULL)
		{
			if(tmp[2] == '\r' && tmp[3] == '\n')
			{
				lagCursor = (tmp - s_buffer) + 4; // set pointer to after the header
				break;
			}
			if(tmp[3] == '\r' && tmp[4] == '\n')
			{
				lagCursor = (tmp - s_buffer) + 5; // set pointer to after the header
				break;
			}
			
			tmp += 2;
			tmp = strstr(tmp, "\r\n");
		}
		
		if(tmp != NULL)
		{
			// we have the header
			
			char *str = (char *)safeMalloc(ICY_HEADER_SIZE);
			memset(str, 0, ICY_HEADER_SIZE);			
			strncpy(str, s_buffer, lagCursor);
			
			parseIcyHeader(str, &curIcy);
			
			streamMode = STREAM_BUFFERING;
			
			// if we have a content type, make sure streaming matches it
			if(strlen(curIcy.icyMimeType) > 0)
			{
				/*
				if(strcmp(curIcy.icyMimeType, "audio/mpeg") == 0){
					TGDSIPC->sndPlayerCtx.sourceFmt = SRC_STREAM_MP3;
				}
				*/
				if(strcmp(curIcy.icyMimeType, "audio/ogg") == 0 || strcmp(curIcy.icyMimeType, "audio/ogg-vorbis") == 0 || strcmp(curIcy.icyMimeType, "audio/vorbis") == 0)
					TGDSIPC->sndPlayerCtx.sourceFmt = SRC_STREAM_OGG;
				if(strcmp(curIcy.icyMimeType, "audio/aac") == 0 || strcmp(curIcy.icyMimeType, "audio/aacp") == 0)
					TGDSIPC->sndPlayerCtx.sourceFmt = SRC_STREAM_AAC;
			}
			
			s_metaCount = 0;
			icyCopy = 0;
			
			// if we have metadata, recieve it!
			if(curIcy.icyMetaInt)
			{	
				int metaSize = ((char *)s_buffer)[lagCursor];
				
				metaSize *= 16;
				icyCopy = 0;
				
				lagCursor++;
				
				if(metaSize > 0)
				{
					icyCopy = metaSize;
					memset(tmpMeta, 0, 4081);
				}
				
				while(icyCopy)
				{
					tmpMeta[strlen(tmpMeta)] = ((char *)s_buffer)[lagCursor];
					lagCursor++;
					icyCopy--;
				}
				
				parseSongTitle();
				
				s_metaCount = 0;
				icyCopy = 0;
			}
			
			safeFree(str);
		}
	}
	
	safeFree(tcpTemp);
	
	s_count++;
	
	if(s_count > 30)
	{
		s_count = 0;
		wifiClose(s_socket);
		s_retries++;
		
		if(s_retries > 2)
		{
			streamMode = STREAM_FAILEDCONNECT;
		}
		else
		{
			startStream();
			
			if(s_socket == -1)
			{
				streamMode = STREAM_FAILEDCONNECT;
			}
		}
	}
}

void recieveStream(int amount) // recieves an amount if > 0, does nothing if = 0, recieves until nothing left if < 0
{		
	if(amount == 0)
		return;
	
	tmpAmount = amount;
	
	char *tcpTemp = (char *)safeMalloc(STREAM_WIFI_READ_SIZE);
	
	int x = 0;	
	recAmount = 0;
	
	while(true)
	{
		int i = recvData(s_socket,tcpTemp,STREAM_WIFI_READ_SIZE);
		
		if(i == 0) 
		{
			// finished unexpectedly
			streamMode = STREAM_FAILEDCONNECT;
			
			safeFree(tcpTemp);
			return;
		}
		else if(i > 0)
		{
			recAmount += i;
			
			circularCopy(s_buffer, tcpTemp, i, &s_cursor, STREAM_BUFFER_SIZE);
			x += i;
		}
		else
		{
			safeFree(tcpTemp);
			return;
		}
		
		if(amount > 0)
		{
			if(x >= amount)
			{
				safeFree(tcpTemp);
				return;
			}
		}
	}
	
	safeFree(tcpTemp);
}

bool amountLeftOver()
{
	// Exit if we aren't supposed to be here
	switch(TGDSIPC->sndPlayerCtx.sourceFmt)
	{
		case SRC_STREAM_MP3:
		case SRC_STREAM_OGG:
		case SRC_STREAM_AAC:
			break;
		default:
			return false;
	}
	
	int x = s_cursor - lagCursor;
	if(x < 0)
		x += STREAM_BUFFER_SIZE;
	
	int percent = (int)(((double)x / (double)STREAM_CACHE_SIZE) * 100);
	
	if(percent > 200)
		return false;
	
	return true;
}

void startStreamAudio()
{
	switch(TGDSIPC->sndPlayerCtx.sourceFmt)
	{
		case SRC_STREAM_MP3:			
		{	
			mad_stream_init(&Stream);
			mad_frame_init(&Frame);
			mad_synth_init(&Synth);
			mad_timer_reset(&Timer);
			
			setSoundLengthDSO(MP3_WRITE_SIZE);
			mallocData(MP3_WRITE_SIZE);
			
			mp3Buf = (unsigned char *)trackMalloc(STREAM_MP3_READ_SIZE, "mp3 stream buf");
			
			TGDSIPC->sndPlayerCtx.channels = 0;
			
			int retries = 0;
			while(TGDSIPC->sndPlayerCtx.channels != 1 && TGDSIPC->sndPlayerCtx.channels != 2)
			{	
				TGDSIPC->sndPlayerCtx.mp3SampleRate = 0;
				while(TGDSIPC->sndPlayerCtx.mp3SampleRate == 0)
				{
					fillMadBufferStream();
					decodeMadBufferStream(0);
					
					TGDSIPC->sndPlayerCtx.mp3SampleRate = Frame.header.samplerate;
				}
				
				TGDSIPC->sndPlayerCtx.channels = MAD_NCHANNELS(&Frame.header);
				
				retries++;
				
				if(retries > 5)
				{
					streamMode = STREAM_FAILEDCONNECT;
					trackFree(mp3Buf);
					break;
				}
			}
			
			if(streamMode != STREAM_FAILEDCONNECT)
			{
				setSoundInterpolation(2);
				setSoundFrequency(TGDSIPC->sndPlayerCtx.mp3SampleRate);
				
				bufCursor = 0;
				streamOpened = true;
				
				startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
			}
			
			break;
		}
		case SRC_STREAM_OGG:
		{
			ov_callbacks oggCallBacks = {callbacks_read_func_stream, callbacks_seek_func_stream, callbacks_close_func_stream, callbacks_tell_func_stream};			
			oggStreamLoc = 0;
			
			if(ov_open_callbacks(&vf, &vf, NULL, 0, oggCallBacks) < 0)
			{
				streamMode = STREAM_FAILEDCONNECT;
				return;
			}
			
			vorbis_info *vi = ov_info(&vf,-1);
			
			TGDSIPC->sndPlayerCtx.channels = vi->channels;		
			if(TGDSIPC->sndPlayerCtx.channels > 2)
			{
				streamMode = STREAM_FAILEDCONNECT; // bad data
				return;
			}
			
			setSoundInterpolation(1);
			setSoundFrequency(vi->rate);		
			setSoundLengthDSO(OGG_READ_SIZE);
			mallocData(OGG_READ_SIZE);		
			
			bufCursor = 0;
			streamOpened = true;
			
			startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
			break;
		}
		case SRC_STREAM_AAC:
		{
			hAACDecoder = (HAACDecoder *)AACInitDecoder();
			if (!hAACDecoder)
			{
				streamMode = STREAM_FAILEDCONNECT;
				return;
			}
			
			aacReadBuf = (unsigned char *)trackMalloc(AAC_READBUF_SIZE, "aac read stream");
			aacOutBuf = (s16 *)trackMalloc(AAC_OUT_SIZE, "aac out stream");
			aacReadPtr = aacReadBuf;
			aacBytesLeft = 0;
			aacEofReached = 0;
			aacErr = ERR_AAC_INDATA_UNDERFLOW;
			isRawAAC = true;
			
			while(aacErr == ERR_AAC_INDATA_UNDERFLOW)
			{
				aacFillBuffer();
				aacErr = AACDecode(hAACDecoder, &aacReadPtr, &aacBytesLeft, aacOutBuf);
			}
			AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);
			
			if(aacFrameInfo.bitsPerSample != 16) // sorry, unsupported
			{
				if(aacReadBuf)
					trackFree(aacReadBuf);			
				if(aacOutBuf)
					trackFree(aacOutBuf);
					
				aacReadBuf = NULL;
				aacOutBuf = NULL;
				
				streamMode = STREAM_FAILEDCONNECT;
				return;
			}
			
			TGDSIPC->sndPlayerCtx.channels = aacFrameInfo.nChans;	
			if(TGDSIPC->sndPlayerCtx.channels > 2 || TGDSIPC->sndPlayerCtx.channels == 0)
			{
				if(aacReadBuf)
					trackFree(aacReadBuf);			
				if(aacOutBuf)
					trackFree(aacOutBuf);
					
				aacReadBuf = NULL;
				aacOutBuf = NULL;
				
				streamMode = STREAM_FAILEDCONNECT;
				return;
			}
			
			setSoundInterpolation(1);
			setSoundFrequency(aacFrameInfo.sampRateOut);		
			setSoundLengthDSO(aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);
			mallocData(aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);	
			
			memcpy(lBuffer, aacOutBuf, aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels);		
			trackFree(aacOutBuf);		
			aacOutBuf = NULL;
			
			aacLength = aacFrameInfo.outputSamps / TGDSIPC->sndPlayerCtx.channels;
			
			startSound9(TGDSIPC->sndPlayerCtx.sourceFmt);
			streamOpened = true;
			
			break;
		}
	}
}
	
void doCurrentStatus()
{
	/*
	switch(streamMode)
	{
		case STREAM_DISCONNECTED: // start connecting
			connectWifi(); // request connect
			streamMode = STREAM_CONNECTING;
			break;
		case STREAM_CONNECTING: // test status
		{
			int i = Wifi_AssocStatus(); // check status
			if(i==ASSOCSTATUS_ASSOCIATED)
				streamMode = STREAM_CONNECTED;
				
			if(i==ASSOCSTATUS_CANNOTCONNECT) 
				streamMode = STREAM_FAILEDCONNECT;
			
			break;
		}
		case STREAM_CONNECTED:
			fillIPFromServer(&curSite);
			
			freeStreamBuffer();
			allocateStreamBuffer();
			
			s_cursor = 0;
			lagCursor = 0;
			
			startStream();
			s_retries = 0;
			s_count = 0;
			
			streamMode = STREAM_STARTING;
			
			break;
		case STREAM_STARTING:
			checkStream();
			break;
		case STREAM_BUFFERING:
			recieveStream(-1);
			
			if((s_cursor - lagCursor) > STREAM_CACHE_SIZE)
			{
				streamMode = STREAM_STREAMING;
				startStreamAudio();
			}
			break;
		case STREAM_STREAMING:
			if(lagCursor == s_cursor || cutOff == true) // we've run out of buffer
			{
				closeSound();
				streamMode = STREAM_TRYNEXT;
			}
			
			// streaming!
			break;
		case STREAM_FAILEDCONNECT:
			freeStreamBuffer();
			closeSound();
			
			streamMode = STREAM_TRYNEXT;
			// failed :<
			break;
		case STREAM_TRYNEXT:
			// pretty self explanatory
			break;
	}
	*/
}

ICY_HEADER *getStreamData()
{
	return &curIcy;
}

int getCurrentStatus()
{
	doCurrentStatus();
	
	return streamMode;
}

int getSourceFmt()
{
	return TGDSIPC->sndPlayerCtx.sourceFmt;
}

void pauseSound(bool pause)
{
	switch(TGDSIPC->sndPlayerCtx.sourceFmt)
	{
		case SRC_STREAM_MP3:
		case SRC_STREAM_OGG:
		case SRC_STREAM_AAC:
			return;
	}
	
	sndPaused = pause;
}

void getSoundLoc(u32 *loc, u32 *max)
{
	// return a dummy value representing 0 for files that aren't loaded.
	if(TGDSIPC->sndPlayerCtx.sourceFmt == SRC_NONE)
	{
		*loc = 0;
		*max = 1;
		
		return;
	}
	
	// a bunch of just-in-casers to ensure the progress is never drawn weird.
	if(TGDSIPC->sndPlayerCtx.loc < 0)
	{
		*loc = 0;
	}
	else
	{
		u32 tLen = TGDSIPC->sndPlayerCtx.loc;
		
		if(seekUpdate > 0)
			tLen = seekUpdate;
		
		if(tLen > TGDSIPC->sndPlayerCtx.len)
		{
			*loc = TGDSIPC->sndPlayerCtx.len;
		}
		else
		{
			*loc = tLen;
		}
	}
	
	// return the max.
	*max = TGDSIPC->sndPlayerCtx.len;
}

void setSoundLoc(u32 loc)
{
	seekSpecial = true;
	
	switch(TGDSIPC->sndPlayerCtx.sourceFmt)
	{
		case SRC_MIKMOD:
			if(loc < TGDSIPC->sndPlayerCtx.len)
			{
				seekUpdate = loc;
			}
			break;
		case SRC_MP3:
		case SRC_OGG:
			TGDSIPC->sndPlayerCtx.loc = loc;
			
			fseek(TGDSIPC->sndPlayerCtx.filePointer, TGDSIPC->sndPlayerCtx.loc, 0);
			break;
		case SRC_FLAC:
			seekUpdate = loc;
			fseek(TGDSIPC->sndPlayerCtx.filePointer, seekUpdate, 0);
			break;			
		case SRC_AAC:
			seekUpdate = loc;
			fseek(TGDSIPC->sndPlayerCtx.filePointer, seekUpdate, 0);
			break;
	}
	
	seekSpecial = false;
}

void closeSound()
{	
	freeSound();
	
	if(isWIFIConnected())
		disconnectWifi();
	soundLoaded = false;
}

int getState()
{
	if(TGDSIPC->sndPlayerCtx.sourceFmt == SRC_NONE)
		return STATE_UNLOADED;
		
	if(TGDSIPC->sndPlayerCtx.sourceFmt == SRC_STREAM_OGG || TGDSIPC->sndPlayerCtx.sourceFmt == SRC_STREAM_MP3 || TGDSIPC->sndPlayerCtx.sourceFmt == SRC_STREAM_AAC)
	{	
		if(streamMode != STREAM_TRYNEXT)
			return STATE_PLAYING;	
		else
			return STATE_STOPPED;
	}
	else
	{
		if(playing)
		{
			if(sndPaused)
				return STATE_PAUSED;
			else
				return STATE_PLAYING;
		}
	}
	
	return STATE_STOPPED;
}

int getStreamLag()
{
	return lagCursor;
}

int getStreamLead()
{
	return s_cursor;
}

u32 getSoundChannels()
{
	return TGDSIPC->sndPlayerCtx.channels;
}
 
int getSIDTrack()
{
	return sid_subSong + 1;
}

int getSIDTotalTracks()
{
	return sid_subSongsMax + 1;
}

int getGBSTrack()
{
	return gbsTrack + 1;
}

int getGBSTotalTracks()
{
	return info.track_count;
}

char *gbsMeta(int which)
{
	switch(which)
	{
		case 0:
			return info.game;
		case 1:
			return info.author;
		case 2:
			return info.copyright;
		case 3:
			return info.comment;
	}
	
	return 0;
}


void checkEndSound()
{
	
	if(soundLoaded == false)
	{
		/*
		char ext[256];
		char tmp[256];
		
		strcpy(tmp,getFileName());
		separateExtension(tmp,ext);
		strlwr(ext);
		
		if(strcmp(ext,".pls") == 0 || strcmp(ext,".m3u") == 0)
		{	
			sndMode = TYPE_PLS;
			if(loadPlaylist(getFileName(), &curPlaylist))
			{
				loadSound(curPlaylist.urlEntry[0].data);
				plsPos = 0;
			}
			else
			{
				destroyRandomList();
				exitSound(0,0);
				return;
			}
		}
		else
		{
		*/
			//sndMode = TYPE_NORMAL;
			
		//}
		
		//sampleWidth = (getSoundLength() / 236);
	}
	
	/*
	firstTime = false;
	
	if(getState() == STATE_STOPPED || getState() == STATE_UNLOADED)
	{
		if(sndMode == TYPE_NORMAL)
		{
			if(soundMode == SOUND_ONESHOT)
			{
				exitSound(0,0);
			}
			else
			{
				getNextSoundInternal(true);
			}
		}
		if(sndMode == TYPE_PLS)
		{
			if(!queued)
			{
				if(plsPos == curPlaylist.numEntries - 1)
					plsPos = 0;
				else
					plsPos++;
			}
			
			loadSound(curPlaylist.urlEntry[plsPos].data);

			queued = false;
		}		
	}
	*/

}

//Audio commands: drive Sound Player Context (Note: different from soundTGDS.h -> Sound Sample Context)
void setSoundLengthDSO(u32 len)
{
	setSoundLength(len);
	sndLen = len;
}
