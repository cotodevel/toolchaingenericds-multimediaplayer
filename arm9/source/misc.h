#ifdef __cplusplus
extern "C" {
#endif

#ifndef __misc9_h__
#define __misc9_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "fatfslayerTGDS.h"

//current File Selected
extern char fileName[MAX_TGDSFILENAME_LENGTH+1];

//current Directory we at
extern char curDir[MAX_TGDSFILENAME_LENGTH+1];

extern void *trackMalloc(u32 length, char *desc);
extern void trackFree(void *tmp);
extern void *trackRealloc(void *tmp, u32 length);

extern void *safeMalloc(size_t size);
extern void *safeRealloc(void *ptr, size_t size);
extern void* safeCalloc(size_t num, size_t size);
extern void safeFree(void *p);

extern void enableVBlank();
extern void disableVBlank();
extern u32 flength(FILE* fh);
extern void separateExtension(char *str, char *ext);

extern int findValidSockID(int sock);
extern int recvData(int sock, char *data, int len);

#endif

#ifdef __cplusplus
}
#endif
