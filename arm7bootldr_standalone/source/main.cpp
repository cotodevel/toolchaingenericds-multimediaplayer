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

#include "main.h"
#include "biosTGDS.h"
#include "loader.h"
#include "busTGDS.h"
#include "dmaTGDS.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "wifi_arm7.h"
#include "pff.h"
#include "ima_adpcm.h"
#include "soundTGDS.h"
#include "biosTGDS.h"
#include "timerTGDS.h"
#include "InterruptsARMCores_h.h"
#include "loader.h"
#include "ipcfifoTGDSUser.h"
#include "dldi.h"
#include "debugNocash.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////Custom ARM7 VRAM Core/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__attribute__((section(".iwram64K")))
IMA_Adpcm_Player backgroundMusicPlayer;	//Actual PLAYER Instance. See ima_adpcm.cpp -> [PLAYER: section

__attribute__((section(".iwram64K")))
IMA_Adpcm_Player SoundEffect0Player;

__attribute__((section(".iwram64K")))
FATFS fileHandle;					// bootloader / / Sound stream handle

__attribute__((section(".iwram64K")))
FATFS FatfsFILESoundSample0; //Sound effect handle #0

struct soundPlayerContext soundData;
char fname[256];

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void playSoundStreamARM7(){
	uint32 * fifomsg = (uint32 *)NDS_CACHED_SCRATCHPAD;			
	UINT br;
	uint8_t fresult;
	bool loop = fifomsg[34];
	FATFS * currentFH;
	u32 streamType = (u32)fifomsg[35];
	struct sIPCSharedTGDSSpecific* sharedIPC = getsIPCSharedTGDSSpecific();
	char * filename = (char*)&sharedIPC->filename[0];
	strcpy((char*)fname, filename);
	
	if(streamType == FIFO_PLAYSOUNDSTREAM_FILE){
		currentFH = &fileHandle;
	}
	else if(streamType == FIFO_PLAYSOUNDEFFECT_FILE){
		currentFH = &FatfsFILESoundSample0;
	}
	fresult = pf_mount(currentFH);
	if (fresult != FR_OK) { 
		
		//Throw exception
		int stage = 10;
		handleDSInitOutputMessage("playSoundStreamARM7(): pf_mount() failed");
		handleDSInitError7(stage, (u32)savedDSHardware);
		
		fifomsg[33] = 0xAABBCCDD;
	}
	fresult = pf_open(fname, currentFH);
	if(streamType == FIFO_PLAYSOUNDEFFECT_FILE){
		if (fresult != FR_OK) { 
			//soundeffect failed to open
		}
		else{
			//soundeffect open OK
		}
	}
	pf_lseek(0, currentFH);
	
	int argBuffer[MAXPRINT7ARGVCOUNT];
	memset((unsigned char *)&argBuffer[0], 0, sizeof(argBuffer));
	argBuffer[0] = 0xc070ffff;
	
	//decode audio here
	bool loop_audio = loop;
	bool automatic_updates = false;
	if(streamType == FIFO_PLAYSOUNDSTREAM_FILE){
		if(backgroundMusicPlayer.play(loop_audio, automatic_updates, ADPCM_SIZE, stopSoundStreamUser, currentFH, streamType) == 0){
			//ADPCM Playback!
		}
	}
	else if(streamType == FIFO_PLAYSOUNDEFFECT_FILE){
		if(SoundEffect0Player.play(loop_audio, automatic_updates, ADPCM_SIZE, stopSoundStreamUser, currentFH, streamType) == 0){
			//ADPCM Sample Playback!
		}
	}
	fifomsg[33] = (u32)fresult;
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void playerStopARM7(){
	REG_IE&=~IRQ_TIMER2;
	TIMERXDATA(1) = 0;
	TIMERXCNT(1) = 0;
	TIMERXDATA(2) = 0;
	TIMERXCNT(2) = 0;
	for(int ch=0;ch<4;++ch)
	{
		SCHANNEL_CR(ch) = 0;
		SCHANNEL_TIMER(ch) = 0;
		SCHANNEL_LENGTH(ch) = 0;
		SCHANNEL_REPEAT_POINT(ch) = 0;
	}
	
	memset((void *)strpcmL0, 0, 1024);
	memset((void *)strpcmL1, 0, 1024);
	memset((void *)strpcmR0, 0, 1024);
	memset((void *)strpcmR1, 0, 1024);

}

void stopBGMusic7(){
	backgroundMusicPlayer.stop();
}

bool stopSoundStreamUser(){

}

void bootfile(){
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////TGDS-mb v3 end /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	/*			TGDS 1.6 Standard ARM7 Init code start	*/
	installWifiFIFO();
	while(!(*(u8*)0x04000240 & 2) ){} //wait for VRAM_D block
	ARM7InitDLDI(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, TGDSDLDI_ARM7_ADDRESS);
	SendFIFOWords(FIFO_ARM7_RELOAD, 0xFF); //ARM7 Reload OK -> acknowledge ARM9
    /*			TGDS 1.6 Standard ARM7 Init code end	*/
	REG_IE|=(IRQ_VBLANK); //X button depends on this
	
	while (1) {
		handleARM7SVC();	/* Do not remove, handles TGDS services */
		HaltUntilIRQ(); //Save power until next Vblank
	}
	return 0;
}
