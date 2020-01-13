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

//This file abstracts specific TGDS console code which allows for easy DS console setup.

#include "gui_console_connector.h"
#include "consoleTGDS.h"

////////[For custom Console implementation]:////////
//You need to override :
	//ConsoleInstance * getProjectSpecificVRAMSetup()
	//Which provides a proper custom 2D VRAM setup

//Then override :
	//bool InitProjectSpecificConsole(ConsoleInstance * ConsoleInstanceInst)
	//Which provides the console init code, example not available here, checkout projects that support Custom console implementation.

//After that you can call :
	//bool project_specific_console = true;
	//GUI_init(project_specific_console);


////////[For default Console implementation simply call]:////////
	//bool project_specific_console = false;
	//GUI_init(project_specific_console);


	////////[Custom Console implementation]////////


//Definition that overrides the weaksymbol expected from toolchain to init console video subsystem
ConsoleInstance * getProjectSpecificVRAMSetup(){
	return DEFAULT_CONSOLE_ENGINE_A_VRAMSETUP();
}


//Generic console (uses VRAM block C,VRAM block D for ARM7) on MAIN Engine/Engine A.

//1) VRAM Layout
ConsoleInstance * DEFAULT_CONSOLE_ENGINE_A_VRAMSETUP(){
	ConsoleInstance * CustomSessionConsoleInst = (ConsoleInstance *)(&CustomConsole);
	memset (CustomSessionConsoleInst, 0, sizeof(ConsoleInstance));
	vramSetup * vramSetupInst = (vramSetup *)&CustomSessionConsoleInst->thisVRAMSetupConsole;
	
	vramSetupInst->vramBankSetupInst[VRAM_A_INDEX].vrambankCR = VRAM_A_0x06000000_ENGINE_A_BG;	//console here
	vramSetupInst->vramBankSetupInst[VRAM_A_INDEX].enabled = true;
	
	vramSetupInst->vramBankSetupInst[VRAM_C_INDEX].vrambankCR = VRAM_C_0x06200000_ENGINE_B_BG;	//NDS BMP rgb15 mode + keyboard
	vramSetupInst->vramBankSetupInst[VRAM_C_INDEX].enabled = true;
	
	// 128K ARM7
	vramSetupInst->vramBankSetupInst[VRAM_D_INDEX].vrambankCR = VRAM_D_0x06000000_ARM7;
	vramSetupInst->vramBankSetupInst[VRAM_D_INDEX].enabled = true;
	
	return CustomSessionConsoleInst;
}



//2) Uses mainEngine: VRAM Layout -> Console Setup
bool InitProjectSpecificConsole(ConsoleInstance * ConsoleInstanceInst){
	
	//Set mainEngine
	SetEngineConsole(mainEngine,ConsoleInstanceInst);
	
	//Set mainEngine properties
	ConsoleInstanceInst->ConsoleEngineStatus.ENGINE_DISPCNT	=	(uint32)(MODE_5_2D | DISPLAY_BG3_ACTIVE );
	
	// BG3: FrameBuffer : 64(TILE:4) - 128 Kb
	ConsoleInstanceInst->ConsoleEngineStatus.EngineBGS[3].BGNUM = 3;
	ConsoleInstanceInst->ConsoleEngineStatus.EngineBGS[3].REGBGCNT = BG_BMP_BASE(4) | BG_BMP8_256x256 | BG_PRIORITY_1;
	
	GUI.DSFrameBuffer = (uint16 *)BG_BMP_RAM(4);	//0x06000000
	
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1 << 8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1 << 8;
	
	BG_PALETTE[0] = RGB15(0,0,0);			//back-ground tile color
	BG_PALETTE[255] = RGB15(31,31,31);		//tile color
	
	UpdateConsoleSettings(ConsoleInstanceInst);
	
	SWAP_LCDS();	//this custom console setting moves the console to top screen, thus, move it back
	return true;
}