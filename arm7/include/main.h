#ifndef __main7_h__
#define __main7_h__

typedef void (*type_void)();
#include "spcdefs.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

// Play buffer, left buffer is first MIXBUFSIZE * 2 uint16's, right buffer is next
extern uint16 * playBuffer; //[MIXBUFSIZE * 2 * 2]
extern volatile int soundCursor;
extern bool paused;
extern bool SPCExecute;
extern void SetupSoundSPC();
extern void StopSoundSPC();
extern int PocketSPCVersion;
extern void LoadSpc(const uint8 *spc);

extern void taskA(u32 * args);
extern void handleTurnOnTurnOffTouchscreenTimeout();
extern void onThreadOverflowUserCode7(u32 * args);
extern u16 keyPressTGDSProject7;
extern bool TSCKeyActive;
#ifdef __cplusplus
}
#endif

