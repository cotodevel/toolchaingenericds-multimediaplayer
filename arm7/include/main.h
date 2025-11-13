#ifndef __main7_h__
#define __main7_h__

#include "spcdefs.h"

typedef void (*type_void)();

#endif


#ifdef __cplusplus
extern "C" {
#endif

// Play buffer, left buffer is first MIXBUFSIZE * 2 uint16's, right buffer is next
extern uint16 * playBuffer; //[MIXBUFSIZE * 2 * 2]
extern volatile int soundCursor;
extern int apuMixPosition;
extern int pseudoCnt;
extern int frame;
extern uint32 interrupts_to_wait_arm7;
extern bool paused;
extern bool SPC_disable;
extern bool SPC_freedom;

extern void SetupSoundSPC();
extern void StopSoundSPC();
extern void LoadSpc(const uint8 *spc);
extern void SaveSpc(uint8 *spc);
extern bool SPCExecute;
extern int PocketSPCVersion;

extern void taskA(u32 * args);
extern void handleTurnOnTurnOffTouchscreenTimeout();
extern void onThreadOverflowUserCode7(u32 * args);
extern u16 keyPressTGDSProject7;
extern bool TSCKeyActive;

#ifdef __cplusplus
}
#endif

