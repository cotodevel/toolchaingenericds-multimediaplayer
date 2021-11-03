
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

//TGDS required version: IPC Version: 1.3

//IPC FIFO Description: 
//		struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress; 														// Access to TGDS internal IPC FIFO structure. 		(ipcfifoTGDS.h)
//		struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;		// Access to TGDS Project (User) IPC FIFO structure	(ipcfifoTGDSUser.h)


#include "ipcfifoTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"

#ifdef ARM7
#include <string.h>

#include "main.h"
#include "spifwTGDS.h"
#include "click_raw.h"

#endif

#ifdef ARM9

#include <stdbool.h>
#include "main.h"
#include "sound.h"

#endif

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoNotEmptyWeakRef(volatile u32 cmd1, volatile u32 cmd2){
	switch (cmd1) {
		//NDS7: 
		#ifdef ARM7
		case(FIFO_TGDSAUDIOPLAYER_ENABLEIRQ):{
			REG_DISPSTAT = (DISP_VBLANK_IRQ | DISP_YTRIGGER_IRQ);
			REG_IE = REG_IE | (IRQ_VBLANK|IRQ_VCOUNT);
		}
		break;
		
		case(FIFO_TGDSAUDIOPLAYER_DISABLEIRQ):{
			REG_DISPSTAT = 0;
			REG_IE = REG_IE & ~(IRQ_VBLANK|IRQ_VCOUNT);
		}
		break;
		#endif
		
		//NDS9: 
		#ifdef ARM9
		#endif
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2){
}

#ifdef ARM9
void enableFastMode(){
	SendFIFOWords(FIFO_TGDSAUDIOPLAYER_DISABLEIRQ, 0xFF);
}

void disableFastMode(){
	SendFIFOWords(FIFO_TGDSAUDIOPLAYER_ENABLEIRQ, 0xFF);
}
#endif