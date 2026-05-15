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
#include "timerTGDS.h"

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
	if(TSCKeyActive == true){
		//TGDS Touchscreen handling X/Y/Touchscreen
	}
	else{
		//If touchscreen disabled, still get X/Y/Touchscreen keys (but no TSC coords)
		u16 keys= keyPressTGDSProject7;
		struct sIPCSharedTGDS * sIPCSharedTGDSInst = (struct sIPCSharedTGDS *)TGDSIPCStartAddress;
		struct touchPosition * sTouchPosition = (struct touchPosition *)&sIPCSharedTGDSInst->tscIPC;
		
		//ARM7 Keypad has access to X/Y/Hinge/Pen down bits
		sIPCSharedTGDSInst->KEYINPUT7 = (uint16)REG_KEYINPUT;
		sIPCSharedTGDSInst->buttons7	= keys;
	}
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

bool timer1PlaybackARM7SPCCore = false;
//TGDS SDK Code. Since the ARM7 Audio Stream + SPC Core has ran out of memory, just paste the bits required for the Audio Stream Code istead of linking the whole library which doesn't fit in the ARM7 IWRAM
void TIMER1Handler(){
	if(SPCExecute == false){
		
		if(timer1PlaybackARM7SPCCore == true){
			setSwapChannel(); //causes buzz
		}

		SendFIFOWords(ARM9COMMAND_UPDATE_BUFFER, 0xFF);
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void setSwapChannel() 
{
	s16 *buf;
  
	if(!sndCursor)
		buf = strpcmL0;
	else
		buf = strpcmL1;
    
	// Left channel
	SCHANNEL_SOURCE((sndCursor << 1)) = (uint32)buf;
	SCHANNEL_CR((sndCursor << 1)) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_16BIT;
    	
	if(!sndCursor)
		buf = strpcmR0;
	else
		buf = strpcmR1;
	
	// Right channel
	SCHANNEL_SOURCE((sndCursor << 1) + 1) = (uint32)buf;
	SCHANNEL_CR((sndCursor << 1) + 1) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x3FF) | SOUND_16BIT;
  
	sndCursor = 1 - sndCursor;
}

void stopSound() {
	TIMERXCNT(0) = 0;
	TIMERXCNT(1) = 0;
	
	SCHANNEL_CR(0) = 0;
	SCHANNEL_CR(1) = 0;
	SCHANNEL_CR(2) = 0;
	SCHANNEL_CR(3) = 0;
	
	REG_IE &= ~IRQ_TIMER1;
	timer1PlaybackARM7SPCCore = false;
}

void setupSound(uint32 sourceBuf) {
	
	dmaFillHalfWord(0, 0, (uint32)APU_RAM_ADDRESS, (uint32)(64*1024) );

	//Init SoundSampleContext
	initSound();

	sndCursor = 0;
	if(multRate != 1 && multRate != 2 && multRate != 4){
		multRate = 1;
	}
	
	mallocDataARM7(sampleLen * 2 * multRate, (uint16*)sourceBuf);
    
	TIMERXDATA(0) = SOUND_FREQ((sndRate * multRate));
	TIMERXCNT(0) = TIMER_DIV_1 | TIMER_ENABLE;
  
	TIMERXDATA(1) = 0x10000 - (sampleLen * 2 * multRate);
	TIMERXCNT(1) = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
	
	int ch;
	for(ch=0;ch<4;++ch)
	{
		SCHANNEL_CR(ch) = 0;
		SCHANNEL_TIMER(ch) = SOUND_FREQ(sndRate * multRate);
		SCHANNEL_LENGTH(ch) = (sampleLen * multRate) >> 1;
		SCHANNEL_REPEAT_POINT(ch) = 0;
	}

	REG_IE |= IRQ_TIMER1;
	
	// prevent accidentally reading garbage from buffer 0, by waiting for buffer 1 instead
	swiDelay((0x10000 - (sampleLen * multRate)) >> 1);
	
	lastL = 0;
	lastR = 0;
}

void mallocDataARM7(int size, uint16* sourceBuf)
{
    // this no longer uses malloc due to using dynamic memory.
	strpcmL0 = (s16*)sourceBuf;
	strpcmL1 = strpcmL0 + (size >> 1);
	strpcmR0 = strpcmL1 + (size >> 1);
	strpcmR1 = strpcmR0 + (size >> 1);
	
	// clear memory to not have sound leftover
	dmaFillHalfWord(0, 0, (uint32)strpcmL0, (uint32)((size + 3) & ~3) );
	dmaFillHalfWord(0, 0, (uint32)strpcmR0, (uint32)((size + 3) & ~3) );
}