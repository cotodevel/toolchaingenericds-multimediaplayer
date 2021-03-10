#ifndef __misc9_h__
#define __misc9_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "fatfslayerTGDS.h"
#include "http.h"

#endif

#ifdef __cplusplus
extern "C"{
#endif

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

//DS Networking: either one of these modes.

extern void initWifi();

//mode nifi local
extern void connectNifiLocal();

//mode nifi over internet
extern void connectNifiOverInternet();

//disconnect nifi
extern void disconnectNifi();

//Wifi AP Internet
extern void disconnectWifi();
extern void connectWifi();
extern bool isWIFIConnected();
extern bool wifiEnabled;

extern void wifiClose(int sock);
extern int findValidSockID(int sock);
extern int recvData(int sock, char *data, int len);

extern int sendStreamRequest(u32 ip, unsigned short port, char *remotePath, bool useMeta);
extern bool parseIcyHeader(char *header, ICY_HEADER *outFile);

#ifdef __cplusplus
}
#endif
