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
#include "interrupts.h"
#include "biosTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "spifwTGDS.h"
#include "wifi_arm7.h"
#include "main.h"

//User Handler Definitions
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void IpcSynchandlerUser(){
	uint8 ipcByte = receiveByteIPC();
	switch(ipcByte){
		//External ARM Core's sendMultipleByteIPC(uint8 inByte0, uint8 inByte1, uint8 inByte2, uint8 inByte3) received bytes:
		case(IPC_SEND_MULTIPLE_CMDS):{
			struct sIPCSharedTGDS * TGDSIPC = TGDSIPCStartAddress;
			uint8 * ipcMsg = (uint8 *)&TGDSIPC->ipcMesaggingQueue[0];
			#ifdef ARM9
			coherent_user_range_by_size((uint32)ipcMsg, sizeof(TGDSIPC->ipcMesaggingQueue));
			#endif
			uint8 inByte0 = (u8)ipcMsg[0];
			uint8 inByte1 = (u8)ipcMsg[1];
			uint8 inByte2 = (u8)ipcMsg[2];
			uint8 inByte3 = (u8)ipcMsg[3];
			
			//Do stuff.
			ipcMsg[3] = ipcMsg[2] = ipcMsg[1] = ipcMsg[0] = 0;
		}
		break;
		
		default:{
			//ipcByte should be the byte you sent from external ARM Core through sendByteIPC(ipcByte);
		}
		break;
	}
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
	TIMER1Handler();	//Audio playback handler
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
	SetLedState(LED_ON);
	isArm7ClosedLid = false;
}

//Note: this event is hardware triggered from ARM7, on ARM9 a signal is raised through the FIFO hardware
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
inline __attribute__((always_inline)) 
void screenLidHasClosedhandlerUser(){
	setBacklight(0);
	SetLedState(LED_LONGBLINK);
}
