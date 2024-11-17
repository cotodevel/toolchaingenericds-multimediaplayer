#ifndef _DS7_IMA_ADPCM_H
#define _DS7_IMA_ADPCM_H

#include "typedefsTGDS.h"
#include <stdio.h>
#include "soundTGDS.h"
#include "petitfs-src/pff.h"

#define ADPCM_SIZE (int)(2048)		//TGDS IMA-ADPCM buffer size
typedef bool (*closeSoundHandle)();	//ret: true = closed sound stream. false = couldn't close sound stream

enum
{
	state_beginblock,
	state_decode
};

enum {
	WAV_FORMAT_PCM			= 0x1,
	WAV_FORMAT_IMA_ADPCM	= 0x11
};

enum {
	IMA_ADPCM_OKAY,
	IMA_ADPCM_ERROR_CANNOTOPEN,
	IMA_ADPCM_ERROR_NOTRIFFWAVE,
	IMA_ADPCM_ERROR_UNSUPPORTED,
	IMA_ADPCM_ERROR_INVALIDCHANNELS
};

//**********************************************************************************************
//**********************************************************************************************
//**********************************************************************************************

typedef struct t_IMA_Adpcm_Data {

	int curSamps;
	int data1, step1, samp1;
	int data2, step2, samp2;
} IMA_Adpcm_Data;

//**********************************************************************************************
//**********************************************************************************************
//**********************************************************************************************

#ifdef __cplusplus

class IMA_Adpcm_Stream
{
private:
	IMA_Adpcm_Data		data;
	IMA_Adpcm_Data		loop_data;
	
	u8		datacache[ADPCM_SIZE];	//Highest IMA-ADPCM block is 512 bytes but we add some overhead just in case
	u8*		loop_src;
	int		loop_cblock;
	int		loop_state;
	int		loop_br;
	int		loop_oddnibble;
	int		wave_data;
	int		wave_end;
	u8		*srcb;
	u8		*src;
	int		oddnibble;
	int		block;
	int		blockremain;
	int		loop1, loop2;
	int		channels;
	int		position;
	int		currentblock;
	int		state;
	int		format;
	int		sampBits;
	int		sampling_rate;
	int		is_processing;
	
	void	open( const void* );
	void	skip( int );
	void	seek( int );
	int		tell();
	int		fget8();
	int		fget16();
	u32		fget32();
	void	getblock();
	int		get8();
	int		get16();
	u32		get32();
	
	void	capture_frame();
	void	restore_frame();
public:
	FATFS * currentFatfsFILEHandle;
	int		wave_loop;
	IMA_Adpcm_Stream();
	~IMA_Adpcm_Stream();
	int reset(bool loop);
	int stream( s16 *target, int length);
	void close();
	int get_format();
	int		stream_pcm( s16 *target, int length );
	int		decode_ima( s16 *target, int length );
	int get_channels();
	int get_sampling_rate();
	closeSoundHandle closeCb;
};

//**********************************************************************************************
//**********************************************************************************************
//**********************************************************************************************

class IMA_Adpcm_Player {
	bool autofill;
	bool paused;
	int currentStreamingMode;
public:
	IMA_Adpcm_Stream stream;
	bool active;
	IMA_Adpcm_Player();
	wavFormatChunk headerChunk;
	int play(bool loop_audio, bool automatic_updates, int buffer_length = ADPCM_SIZE / 8, closeSoundHandle = NULL, FATFS * inFatfsFILEHandle = NULL, int incomingStreamingMode = 0);
	void pause();
	void resume();
	void stop();
	void setvolume( int vol );
	int getvolume();
	bool isactive();
	int i_stream_request( int length, void * dest, int frmt );
	IMA_Adpcm_Stream * getStream();
	// update function for manual filling
	void update();
};
#endif

#endif

#ifdef __cplusplus
extern "C" {
extern void IMAADPCMDecode(s16 * lBuf, s16 * rBuf, IMA_Adpcm_Player * thisPlayer);
#endif

extern int ADPCMchannels;

extern void SendArm7Command(u32 command, u32 data);
extern void swapData();

extern bool player_loop;
extern void soundPauseStart();
extern void timerAudioCallback();
extern void setupSoundTGDSVideoPlayerARM7();
extern u8 adpcmWorkBuffer[ADPCM_SIZE*2];
extern u8 streamBuffer[ADPCM_SIZE*2];

#ifdef __cplusplus
}
#endif