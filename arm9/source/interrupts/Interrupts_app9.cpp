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

#include "InterruptsARMCores_h.h"
#include "../../../common/ipcfifoTGDSUser.h"
#include "dsregs_asm.h"
#include "../../include/main.h"
#include "keypadTGDS.h"
#include "../../include/interrupts.h"
#include "utilsTGDS.h"
#include "spifwTGDS.h"
#include "powerTGDS.h"

//User Handler Definitions
#include "TGDSVideo.h"
#include "woopsifuncs.h"
#include "WoopsiTemplate.h"

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void IpcSynchandlerUser(uint8 ipcByte){
	switch(ipcByte){
		default:{
			//ipcByte should be the byte you sent from external ARM Core through sendByteIPC(ipcByte);
		}
		break;
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer0handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer1handlerUser(){
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer2handlerUser(){
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void Timer3handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void HblankUser(){
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VblankUser(){
	if(TGDSVideoPlayback == true){
		if(vblankCount < vblankMaxFrame){
			vblankCount++;
		}
		else{
			vblankCount = 1;
		}
	}
	woopsiVblFunc();
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void VcounterUser(){
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void screenLidHasOpenedhandlerUser(){
	if(WoopsiTemplateProc != NULL){
		WoopsiTemplateProc->handleLidOpen();
	}
	setBacklight(POWMAN_BACKLIGHT_BOTTOM_BIT); 
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
void screenLidHasClosedhandlerUser(){
	if(WoopsiTemplateProc != NULL){
		WoopsiTemplateProc->handleLidClosed();
	}
}