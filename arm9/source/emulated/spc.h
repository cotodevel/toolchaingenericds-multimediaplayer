#ifndef _SPC_INCLUDED
#define _sID_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#define ROCKBOX_LITTLE_ENDIAN
#define CPU_ARM

bool spcInit(uint8_t* buffer, u32 buffersize);
bool spcPlay(s16 *lBuf, s16 *rBuf);
void spcFree();
char *getSPCMeta(int which);

#ifdef __cplusplus
}
#endif

#endif
