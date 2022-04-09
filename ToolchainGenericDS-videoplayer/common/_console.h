#ifndef __console_h__
#define __console_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"

#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM7
extern void _consolePrint(char *chr, int argvCount, int * argv);	//TGDS implementation
#endif

#ifdef ARM9
extern void _consolePrint(const char* s);	//TGDS implementation
extern void _consolePrintf(const char* format, ...);
#endif

extern void _consoleLogPause(void);
extern void _consoleLogResume(void);
extern void ShowLogHalt();

#ifdef __cplusplus
}
#endif
