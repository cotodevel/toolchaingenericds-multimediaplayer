/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include "pff.h"
#include "diskio_petit.h"
#ifdef ARM7
#include "dldi.h"
#endif
#ifdef WIN32
#include "../dldi.h"
#endif
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

//Coto- 18 Nov. 2024: Add cached sectors, to remove some more audio clicks!
struct dldiCache dldiCached[MAX_ENTRIES_BUFFERED];
static int sequentialCachedEntry = 0;

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
#ifdef ARM7
__attribute__ ((optnone))
#endif
DSTATUS disk_initialize (void){
	#ifdef WIN32
	if(dldi_handler_init() == true){
	#endif
	#ifdef ARM7
	if(1 == 1){
		memset((u8*)&dldiCached, 0, sizeof(dldiCached));
		int i = 0;
		for(i = 0; i < MAX_ENTRIES_BUFFERED; i++){
			dldiCached[i].sector = -1;
		}
		sequentialCachedEntry = 0;
	#endif
		return 0;
	}
	return FR_DISK_ERR;
}

/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

#ifdef ARM7
__attribute__ ((optnone))
#endif
__attribute__((section(".iwram64K")))
DRESULT disk_readp (
	BYTE* buff,		/* Pointer to the destination object */
	DWORD sector,	/* Sector number (LBA) */
	UINT offset,	/* Offset in the sector */
	UINT count		/* Byte count (bit15:destination) */
)
{
	char * targetBuffer = NULL;
	bool found = false;
	int i = 0;
	for(i = 0; i < MAX_ENTRIES_BUFFERED; i++){
		if(dldiCached[i].sector == sector){
			found = true;
			break;
		}
	}
	if(found == false){
		//Allocate a new entry.
		for(i = 0; i < MAX_ENTRIES_BUFFERED; i++){
			if(dldiCached[i].sector == -1){
				targetBuffer = (char*)&dldiCached[i].scratchPadSector[0];
				dldiCached[i].sector = sector;
				break;
			}
		}
		
		//Ran out of entries? re-use a sequential entry
		if(i >= MAX_ENTRIES_BUFFERED){
			targetBuffer = (char*)&dldiCached[sequentialCachedEntry].scratchPadSector[0];
			dldiCached[sequentialCachedEntry].sector = sector;
			if(sequentialCachedEntry < (MAX_ENTRIES_BUFFERED-1)){
				sequentialCachedEntry++;
			}
			else{
				sequentialCachedEntry=0;
			}
		}
		
		//Perform DLDI sector fetch
		dldi_handler_read_sectors(sector, 1, (char*)targetBuffer);
	}
	
	//Found? Reuse cached entry
	else{
		targetBuffer = (char*)&dldiCached[i].scratchPadSector[0];
	}
	
	memcpy(buff, (char*)&targetBuffer[offset], count);
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/
/*
DRESULT disk_writep (
	const BYTE* buff,	// Pointer to the data to be written, NULL:Initiate/Finalize write operation 
	DWORD sc		// Sector number (LBA) or Number of bytes to send 
)
{
	DRESULT res;


	if (!buff) {
		if (sc) {

			// Initiate write process

		} else {

			// Finalize write process

		}
	} else {

		// Send data to the disk

	}

	return res;
}
*/
