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
#include "ipcfifoTGDSUser.h"
#include "dsregs_asm.h"
#include "main.h"
#include "keypadTGDS.h"
#include "interrupts.h"
#include "utilsTGDS.h"
#include "spifwTGDS.h"

//User Handler Definitions

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void IpcSynchandlerUser(){
	
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Timer0handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Timer1handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Timer2handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void Timer3handlerUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void HblankUser(){
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void VblankUser(){
	struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;
	if(TGDSUSERIPC->frameCounter9 < 60){
		TGDSUSERIPC->frameCounter9++;
	}
	else{
		TGDSUSERIPC->frameCounter9 = 0;
	}
}

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void VcounterUser(){
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void screenLidHasOpenedhandlerUser(){
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);	//both lit screens
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void screenLidHasClosedhandlerUser(){
	
}