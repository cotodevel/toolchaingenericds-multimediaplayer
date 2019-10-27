#include "misc.h"
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDSUser.h"
#include "main.h"
#include "dswnifi_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char fileName[MAX_TGDSFILENAME_LENGTH+1];
char curDir[MAX_TGDSFILENAME_LENGTH+1];

void *safeMalloc(size_t size)
{
	void *tmp = malloc(size);
	if(!tmp)
	{
		//rewind memory
		//sbrk(-(512*1024));	//whatever, just rewind the malloc
		//tmp = calloc(1, size);
	}
	else{
		memset(tmp, 0, size);
	}
	return tmp;
}

void *safeRealloc(void *ptr, size_t size){
	void *tmp = realloc(ptr, size);
	if(!tmp)
	{
		//rewind memory
		//sbrk(-(512*1024));	//whatever, just rewind the malloc 
		//tmp = calloc(1, size);
	}
	return tmp;
}

void* safeCalloc (size_t num, size_t size){
	void *tmp = calloc(num, size);
	if(!tmp)
	{
		//rewind memory
		//sbrk(-(768*1024));	//whatever, just rewind the malloc so we have 1.7~ MB free.
		//tmp = calloc(num, size);
	}
	return tmp;
}

void safeFree(void *p){
	free(p);
}

void enableVBlank()
{
    //irqSet(IRQ_VBLANK, vBlank);
	//irqEnable(IRQ_VBLANK);	
	REG_IE|=IRQ_VBLANK;
}

void disableVBlank()
{
	//irqSet(IRQ_VBLANK, 0);
	//irqDisable(IRQ_VBLANK);	
	REG_IE&=~IRQ_VBLANK;
}

u32 flength(FILE* fh){
	if(fh != NULL){
		u32 cPos = ftell(fh);
		fseek(fh, 0, SEEK_END);
		u32 aPos = ftell(fh);
		fseek(fh, cPos, SEEK_SET);
		return aPos;		
	}
	return 0;
}

void separateExtension(char *str, char *ext)
{
	int x = 0;
	
	for(int y = strlen(str) - 1; y > 0; y--)
	{
		if(str[y] == '.')
		{
			// found last dot
			x = y;
			break;
		}
		if(str[y] == '/')
		{
			// found a slash before a dot, no ext
			ext[0] = 0;
			return;
		}
	}
	
	if(x > 0)
	{
		int y = 0;
		while(str[x] != 0)
		{
			ext[y] = str[x];
			str[x] = 0;
			x++;
			y++;
		}
		ext[y] = 0;
	}
	else
		ext[0] = 0;	
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
	return recv(sock, data, len, 0);
}