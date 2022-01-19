#ifndef _SPC_INCLUDED
#define _SPC_INCLUDED

#define SPC_FREQ 32000
#define SPC_OUT_SIZE 2048

#define lBufferARM7 (s16*)((int)0x037f8000)	
#define rBufferARM7 (s16*)((int)lBufferARM7 + (int)(16*1024))

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define ROCKBOX_LITTLE_ENDIAN
#define CPU_ARM

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern bool spcInit(uint8_t* buffer, u32 buffersize);
extern bool spcPlay(s16 *lBuf, s16 *rBuf);
extern void spcFree();
extern char *getSPCMeta(int which);
extern void spcDecode();

#ifdef __cplusplus
}
#endif