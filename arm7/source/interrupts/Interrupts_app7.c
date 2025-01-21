#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDSUser.h"
#include "InterruptsARMCores_h.h"
#include "spcdefs.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "biosTGDS.h"

//User Handler Definitions

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void IpcSynchandlerUser(uint8 ipcByte){
	switch(ipcByte){
		default:{
			//ipcByte should be the byte you sent from external ARM Core through sendByteIPC(ipcByte);
		}
		break;
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer0handlerUser(){
}

static int timerTick=0;

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer1handlerUser(){
	
}


static int cyclesToExecute = spcCyclesPerSec / (MIXRATE / MIXBUFSIZE);
		
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer2handlerUser(){
	if(SPCExecute == true){
		soundCursor = MIXBUFSIZE - soundCursor;
		s16 * leftOutputChannel = (s16 *)&(playBuffer[MIXBUFSIZE - soundCursor]);
		s16 * rightOutputChannel = (s16 *)&(playBuffer[(MIXBUFSIZE - soundCursor) + (MIXBUFSIZE * 2)]);

		// Left channel
		int channel = soundCursor == 0 ? 0 : 1;
		SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
		SCHANNEL_SOURCE(channel) = (uint32)leftOutputChannel;
		SCHANNEL_LENGTH(channel) = (MIXBUFSIZE * 2) >> 2;
		SCHANNEL_REPEAT_POINT(channel) = 0;
		SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_16BIT;

		// Right channel
		channel = soundCursor == 0 ? 2 : 3;
		SCHANNEL_TIMER(channel) = SOUND_FREQ(MIXRATE);
		SCHANNEL_SOURCE(channel) = (uint32)rightOutputChannel;
		SCHANNEL_LENGTH(channel) = (MIXBUFSIZE * 2) >> 2;
		SCHANNEL_REPEAT_POINT(channel) = 0;
		SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | SOUND_16BIT;

		DspMixSamplesStereo(MIXBUFSIZE, &playBuffer[soundCursor]);
		ApuExecute(cyclesToExecute * 21);
		if(PocketSPCVersion == 9){
			ApuUpdateTimers(cyclesToExecute);
		}
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer3handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HblankUser(){
	
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VblankUser(){
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VcounterUser(){
	
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void screenLidHasOpenedhandlerUser(){
	
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void screenLidHasClosedhandlerUser(){
	
}