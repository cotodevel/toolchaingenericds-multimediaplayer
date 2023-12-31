#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "misc.h"
#include "InterruptsARMCores_h.h"
#include "../../common/ipcfifoTGDSUser.h"
#include "../include/main.h"
#include "posixHandleTGDS.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char fileName[MAX_TGDSFILENAME_LENGTH+1];
char curDir[MAX_TGDSFILENAME_LENGTH+1];

void *safeMalloc(size_t size){
	void *tmp = TGDSARM9Malloc(size);
	if(!tmp)
	{
		exit(-100);
	}
	else{
		memset(tmp, 0, size);
	}
	return tmp;
}

void *safeRealloc(void *ptr, size_t size){
	void *tmp = TGDSARM9Realloc(ptr, size);
	if(!tmp)
	{
		exit(-101);
	}
	return tmp;
}

void* safeCalloc (size_t num, size_t size){
	void *tmp = TGDSARM9Calloc(num, size);
	if(!tmp)
	{
		exit(-102);
	}
	return tmp;
}

void safeFree(void *p){
	TGDSARM9Free(p);
}

void *trackMalloc(u32 length, char *desc)
{
	void *tmp = safeMalloc(length);
	
	#ifdef MALLOC_TRACK
	
	// add here	
	if(mallocListPtr < 100)
	{	
		mallocList[mallocListPtr].memLocation = (u32)tmp;
		mallocList[mallocListPtr].size = (u32)length;
		strcpy(mallocList[mallocListPtr].description, desc);
		
		mallocListPtr++;	
	}
	
	// return normal
	#endif
	
	return tmp;
}

void trackFree(void *tmp)
{
	#ifdef MALLOC_TRACK
	
	// subtract here	
	for(int i=0;i<mallocListPtr;i++)
	{
		if(mallocList[i].memLocation == (u32)tmp)
		{
			for(int j=i;j<mallocListPtr - 1;j++)
			{
				mallocList[j].memLocation = mallocList[j+1].memLocation;
				mallocList[j].size = mallocList[j+1].size;
				strcpy(mallocList[j].description, mallocList[j+1].description);
			}
			
			mallocListPtr--;
			break;
		}
	}
	
	// do normal
	#endif
	
	safeFree(tmp);
}

void *trackRealloc(void *tmp, u32 length)
{
	#ifdef MALLOC_TRACK
	u32 old = (u32)tmp;
	#endif
	
	void *newTmp = safeRealloc(tmp,length);
	
	if(!newTmp)
	{
		printf("trackRealloc: error re-allocating. ");
		while(1) 
		{ 
			// empty, halting processor
		}
	}
	
	#ifdef MALLOC_TRACK
	for(int i=0;i<mallocListPtr;i++)
	{
		if(mallocList[i].memLocation == old)
		{
			if(newTmp)
			{
				mallocList[i].size = (u32)length;
				mallocList[i].memLocation = (u32)newTmp;
				break;
			}
			else
			{
				// handle safeRealloc of null
				for(int j=i;j<mallocListPtr;j++)
				{
					mallocList[j].memLocation = mallocList[j+1].memLocation;
					mallocList[j].size = mallocList[j+1].size;
					strcpy(mallocList[j].description, mallocList[j+1].description);
				}
				
				mallocListPtr--;
				break;
			}
		}
	}
	#endif
	
	return newTmp;
}

int findValidSockID(int sock)
{
	return -1;
}

int recvData(int sock, char *data, int len)
{	
	int id = findValidSockID(sock);
	return -1;
}

void wifiClose(int sock)
{
	//if(wifiConn[id].isSSL)
	//{
		//CyaSSL_shutdown(wifiConn[id].ssl);	//SSL_shutdown
		//CyaSSL_free(wifiConn[id].ssl);	//SSL_free
		//CyaSSL_CTX_free(wifiConn[id].ctx);	//SSL_CTX_free
	//}
	
	//wifiConn[id].isSSL = false;
	//wifiConn[id].sock = -1;
}


//DS Networking: either one of these modes.

//logic -> enter nifi : disconnectWifi() -> connectNifixxxxx()
//		-> enter wifi : disconnectWifi() -> connectWifi()
//		-> idle:	disconnectWifi()
//mode nifi local
void connectNifiLocal(){
	
}

//mode nifi over internet
void connectNifiOverInternet(){
	
}

//disconnect nifi
void disconnectNifi(){
	
}

//Wifi AP Internet
void disconnectWifi()
{
	wifiEnabled = false;
}

void connectWifi()
{
	
}

bool wifiEnabled = false;
bool isWIFIConnected()
{
	return wifiEnabled;
}

void initWifi()
{
	//wifi/nifi init
	disconnectWifi();
	
}


int sendStreamRequest(u32 ip, unsigned short port, char *remotePath, bool useMeta)
{
	//removed
}

bool parseIcyHeader(char *header, ICY_HEADER *outFile)
{
	//removed
}

void destroyURL(URL_TYPE *site){
	//removed
}