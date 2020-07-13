/*
			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
*/

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
 
#include "main.h"
#include "InterruptsARMCores_h.h"
#include "interrupts.h"

#include "ipcfifoTGDSUser.h"
#include "usrsettingsTGDS.h"
#include "timerTGDS.h"
#include "CPUARMTGDS.h"
#include <dswifi7.h>
#include "wifi_arm7.h"
#include "wifi_shared.h"
#include "spiTGDS.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"
#include "utilsTGDS.h"
#include "soundTGDS.h"
#include "keypadTGDS.h"
#include "posixHandleTGDS.h"
#include "dmaTGDS.h"
#include "biosTGDS.h"

void mallocData(int size)
{
	strpcmL0 = (s16 *)TGDSARM7Malloc(size);
	strpcmL1 = (s16 *)TGDSARM7Malloc(size);
	strpcmR0 = (s16 *)TGDSARM7Malloc(size);
	strpcmR1 = (s16 *)TGDSARM7Malloc(size);
	
	// clear vram d bank to not have sound leftover
	int i = 0;
	
	for(i=0;i<(size);++i)
	{
		strpcmL0[i] = 0;
	}
	
	for(i=0;i<(size);++i)
	{
		strpcmR0[i] = 0;
	}
}

void freeData()
{	
	TGDSARM7Free((u8*)strpcmL0);
	TGDSARM7Free((u8*)strpcmL1);
	TGDSARM7Free((u8*)strpcmR0);
	TGDSARM7Free((u8*)strpcmR1);
}

//---------------------------------------------------------------------------------
int main(int _argc, sint8 **_argv) {
//---------------------------------------------------------------------------------
	/*			TGDS 1.6 Standard ARM7 Init code start	*/
	//wait for VRAM D to be assigned from ARM9->ARM7 (ARM7 has load/store on byte/half/words on VRAM)
	while (!(*((vuint8*)0x04000240) & 0x2));
		
	installWifiFIFO();		
		
	int argBuffer[MAXPRINT7ARGVCOUNT];
	memset((unsigned char *)&argBuffer[0], 0, sizeof(argBuffer));
	argBuffer[0] = 0xc070ffff;
	writeDebugBuffer7("TGDS ARM7.bin Boot OK!", 1, (int*)&argBuffer[0]);
		
	/*			TGDS 1.6 Standard ARM7 Init code end	*/

	SoundPowerON(127);		//volume
    while (1) {
		handleARM7SVC();	/* Do not remove, handles TGDS services */
		IRQWait(IRQ_HBLANK);
	}
   
	return 0;
}

//Custom Button Mapping Handler implementation: IRQ Driven
void CustomInputMappingHandler(uint32 readKeys){
	
}

//Project specific: ARM7 Setup for TGDS sound stream
void initSoundStreamUser(u32 srcFmt){
	
}
