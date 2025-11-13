#include <string.h>
#include "spcdefs.h"
#include "apu.h"
#include "dsp.h"
#include "main.h"
#include "InterruptsARMCores_h.h"
#include "interrupts.h"
#include "usrsettingsTGDS.h"
#include "timerTGDS.h"
#include "powerTGDS.h"
#include "dldi.h"
#include "ipcfifoTGDSUser.h"
#include "TGDS_threads.h"
#include "exceptionTGDS.h"
#include "loader.h"
#include "spitscTGDS.h"

bool SPCExecute=false;

int PocketSPCVersion = 0; //9 = pocketspcv0.9 / 10 = pocketspcv1.0

//TGDS-MB v3 bootloader
void bootfile(){
}

// Play buffer, left buffer is first MIXBUFSIZE * 2 uint16's, right buffer is next
uint16 * playBuffer = (uint16 *)TGDS_ARM7_AUDIOBUFFER_STREAM; //[MIXBUFSIZE * 2 * 2]
volatile int soundCursor;
bool paused = false;

void SetupSoundSPC() {
    soundCursor = 0;

	SoundPowerON(127);		//volume
	
	TIMERXDATA(1) = TIMER_FREQ(MIXRATE);
    TIMERXCNT(1) = TIMER_DIV_1 | TIMER_ENABLE;
    TIMERXDATA(2) = 0x10000 - MIXBUFSIZE;
    TIMERXCNT(2) = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
	irqEnable(IRQ_TIMER2);
	irqDisable(IRQ_TIMER1);
	
    // Timing stuff
    //TIMER2_DATA = 0;
    //TIMER2_CR = TIMER_DIV_64 | TIMER_ENABLE;

    //TIMER3_DATA = 0;
    //TIMER3_CR = TIMER_CASCADE | TIMER_ENABLE;

//    *((vu32*)0x04000510) = (uint32)capBuf;
//    *((vu16*)0x04000514) = (MIXBUFSIZE * 2) >> 1;
//    *((vu16*)0x04000508) = 0x83;
	SPCExecute=true;
	
	// prevent accidentally reading garbage from buffer 0, by waiting for buffer 1 instead
	swiDelay((0x10000 - MIXBUFSIZE) >> 1);
}

void StopSoundSPC() {
    powerOFF(POWER_SOUND);
    REG_SOUNDCNT = 0;
    TIMERXCNT(1) = 0;
    TIMERXCNT(2) = 0;
	irqDisable(IRQ_TIMER2);
	SPCExecute=false;
}

void LoadSpc(const uint8 *spc) {
    int i=0;
    
	apuCacheSamples = 1; //SPCs are cached by default. //Set apuCacheSamples = 0; to titles having playback issues on cached samples.
    PocketSPCVersion = 9; //9 == default SPC timers. 10 == faster SPC timers
	
    if(strncmpi((char*)&spc[0x4E], "Yoshi's Island", 14) == 0){
        PocketSPCVersion = 10;
    }
    else if(strncmpi((char*)&spc[0x4E], "Super Mario World", 17) == 0){
        PocketSPCVersion = 10;
	}
    else if(strncmpi((char*)&spc[0x4E], "Earthbound", 10) == 0){
        PocketSPCVersion = 10;
    }
	else if(strncmpi((char*)&spc[0x4E], "Kirby's Dream Land 3", 20) == 0){
        PocketSPCVersion = 10;
	}
	else if(strncmpi((char*)&spc[0x4E], "Rock 'n' Roll Racing", 20) == 0){
        PocketSPCVersion = 9;
	}
	//MegamanX2 & MegamanX3 has cached samples. Megaman X1 has not.
	else if(
		(!(strncmpi((char*)&spc[0x4E], "Megaman X2", 10) == 0))
		&&
		(!(strncmpi((char*)&spc[0x4E], "Megaman X3", 10) == 0))
		&&
		(strncmpi((char*)&spc[0x4E], "Megaman X", 9) == 0)
	){
		apuCacheSamples = 0;
	}
	
    ApuReset();
    DspReset();

// 0 - A, 1 - X, 2 - Y, 3 - RAMBASE, 4 - DP, 5 - PC (Adjusted into rambase)
// 6 - Cycles (bit 0 - C, bit 1 - v, bit 2 - h, bits 3+ cycles left)
// 7 - Optable
// 8 - NZ

    APU_STATE[0] = spc[0x27]<<24; // A
    APU_STATE[1] = spc[0x28]<<24; // X
    APU_STATE[2] = spc[0x29]<<24; // Y
    SetStateFromRawPSW(APU_STATE, spc[0x2A]);
    APU_SP = 0x100 | spc[0x2B]; // SP
    APU_STATE[5] = APU_STATE[3] + (spc[0x25] | (spc[0x26] << 8)); // PC
    for (i=0; i<=0xffff; i++) APU_MEM[i] = spc[0x100 + i];
    for (i=0; i<=0x7f; i++) {
        DSP_MEM[i] = spc[0x10100 + i];
    }
    for (i=0; i<=0x3f; i++) APU_EXTRA_MEM[i] = spc[0x101c0 + i];

    ApuPrepareStateAfterReload();
    DspPrepareStateAfterReload();
}

bool TSCKeyActive = false;

//---------------------------------------------------------------------------------
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int main(int _argc, char **_argv) {
//---------------------------------------------------------------------------------
	/*			TGDS 1.6 Standard ARM7 Init code start	*/
	while(!(*(u8*)0x04000240 & 2) ){} //wait for VRAM_D block
	ARM7InitDLDI(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, TGDSDLDI_ARM7_ADDRESS);
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	/*			TGDS 1.6 Standard ARM7 Init code end	*/
	
	REG_IPC_FIFO_CR = (REG_IPC_FIFO_CR | IPC_FIFO_SEND_CLEAR);	//bit14 FIFO ERROR ACK + Flush Send FIFO
	REG_IE &= ~(IRQ_VBLANK); //cause sound clicks
	
	update_spc_ports();
	int i = 0; 
    for (i = 0; i < MIXBUFSIZE * 4; i++) {
        playBuffer[i] = 0;
    }
	    
    //Add a timeout-based touchscreen thread 
	{
		keyPressTGDSProject7 = 0xFFFF;
		keyPressTGDSProject7 = keyPressTGDSProject7 & (~(1 << 6));
		TSCKeyActive = true;

        int taskATimeMS = 1; //Task execution requires at least 1ms
        if(registerThread(TGDSThreads, (TaskFn)&taskA, (u32*)NULL, taskATimeMS, (TaskFn)&onThreadOverflowUserCode7, tUnitsMilliseconds) != THREAD_OVERFLOW){
                
        }
    }

    SendFIFOWords(0xFF, 0xFF);

	while(1){
		bool waitForVblank = false;
		int threadsRan = runThreads(TGDSThreads, waitForVblank);
	}

	return 0;
}

//4000136h - NDS7 - EXTKEYIN - Key X/Y Input (R)
//Same as GBA, both ARM7 and ARM9 have keyboard input registers, and each its own keypad IRQ control register.
#define REG_KEYXY 		(*(vuint16*)0x04000136)

static int millisecondsElapsedTSCTimeout = 0;	
u16 keyPressTGDSProject7 = 0;

void taskA(u32 * args){
	handleTurnOnTurnOffTouchscreenTimeout();
}

//called 50 times per second
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void handleTurnOnTurnOffTouchscreenTimeout(){
	millisecondsElapsedTSCTimeout ++;
	
	//Handle touchscreen bottom screen timeout
	if(!(keyPressTGDSProject7 & (1 << 6))){
		//strcpy((char*)0x02000000, "                       ");
		//strcpy((char*)0x02000000, "enabling TSC");

		enableARM7TouchScreen();
		millisecondsElapsedTSCTimeout = 0;
		TSCKeyActive = true;
	}

	if(millisecondsElapsedTSCTimeout < 5){
		millisecondsElapsedTSCTimeout++;
	}
	else{
		//disable tsc here
		if( (TSCKeyActive == true) && (keyPressTGDSProject7 & (1 << 6)) ){ //tsc release
			//strcpy((char*)0x02000000, "                       ");
			//strcpy((char*)0x02000000, "disabling TSC");

			disableARM7TouchScreen();
			millisecondsElapsedTSCTimeout = 0;
			TSCKeyActive = false;
		}
	}
	
	keyPressTGDSProject7 = REG_KEYXY;
}

//////////////////////////////////////////////////////// Threading User code start : TGDS Project specific ////////////////////////////////////////////////////////
//User callback when Task Overflows. Intended for debugging purposes only, as normal user code tasks won't overflow if a task is implemented properly.
//	u32 * args = This Task context
void onThreadOverflowUserCode7(u32 * args){
	struct task_def * thisTask = (struct task_def *)args;
	struct task_Context * parentTaskCtx = thisTask->parentTaskCtx;	//get parent Task Context node 
	
	//Thread overflow! Halt.

	while(1==1){
		HaltUntilIRQ();
	}
}

//////////////////////////////////////////////////////////////////////

//Custom Button Mapping Handler implementation: IRQ Driven
void CustomInputMappingHandler(uint32 readKeys){
	
}

//Project specific: ARM7 Setup for TGDS sound stream
void initSoundStreamUser(u32 srcFmt){
	//SPCExecute=false;
}