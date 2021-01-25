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

////////[For custom Console implementation]:////////
//You need to override :
	//vramSetup * getProjectSpecificVRAMSetup()
	//Which provides a proper custom 2D VRAM setup

//Then override :
	//bool InitProjectSpecificConsole()
	//Which provides the console init code, example not available here, checkout projects that support Custom console implementation.

//After that you can call :
	//bool project_specific_console = true;
	//GUI_init(project_specific_console);


////////[For default Console implementation simply call]:////////
	//bool project_specific_console = false;
	//GUI_init(project_specific_console);





	////////[Default Console implementation is selected, thus stubs are implemented here]////////


//Definition that overrides the weaksymbol expected from toolchain to init console video subsystem
vramSetup * getProjectSpecificVRAMSetup(){
	return TGDSAUDIOPLAYER_2DVRAM_SETUP();
}


//2) Uses subEngine: VRAM Layout -> Console Setup
bool InitProjectSpecificConsole(){
	DefaultSessionConsole = (ConsoleInstance *)(&CustomConsole);
	
	ConsoleInstance * ConsoleInstanceInst = DefaultSessionConsole;
	
	//Set subEngine
	SetEngineConsole(subEngine,ConsoleInstanceInst);
	
	//Set subEngine properties
	ConsoleInstanceInst->ConsoleEngineStatus.ENGINE_DISPCNT	=	(uint32)(MODE_5_2D | DISPLAY_BG3_ACTIVE );
	
	// BG3: FrameBuffer : 64(TILE:4) - 128 Kb
	ConsoleInstanceInst->ConsoleEngineStatus.EngineBGS[3].BGNUM = 3;
	ConsoleInstanceInst->ConsoleEngineStatus.EngineBGS[3].REGBGCNT = BG_BMP_BASE(4) | BG_BMP8_256x256 | BG_PRIORITY_1;
	GUI.DSFrameBuffer = (uint16 *)BG_BMP_RAM_SUB(4);
	
	GUI.Palette = &BG_PALETTE_SUB[0];
	GUI.ScanJoypad = 0;
	GUI.Palette[0] = 	RGB8(0,0,0);			//Back-ground tile color / Black
	GUI.Palette[1] =	RGB8(255, 255, 255); 	//White
	GUI.Palette[2] =  	RGB8(150, 75, 0); 		//Brown
	GUI.Palette[3] =  	RGB8(255, 127, 0); 		//Orange
	GUI.Palette[4] = 	RGB8(255, 0, 255); 		//Magenta
	GUI.Palette[5] = 	RGB8(0, 255, 255); 		//Cyan
	GUI.Palette[6] = 	RGB8(255, 255, 0); 		//Yellow
	GUI.Palette[7] = 	RGB8(0, 0, 255); 		//Blue
	GUI.Palette[8] = 	RGB8(0, 255, 0); 		//Green
	GUI.Palette[9] = 	RGB8(255, 0, 0); 		//Red
	GUI.Palette[0xa] = 	RGB8(128, 128, 128); 	//Grey
	GUI.Palette[0xb] = 	RGB8(240, 240, 240);	//Light-Grey
	
	//Fill the Pallette
	int i = 0;
	for(i=0;i < (256 - 0xb); i++){
		GUI.Palette[i + 0xc] = GUI.Palette[TGDSPrintfColor_White];
	}
	
	bool mainEngine = true;
	setOrientation(ORIENTATION_0, mainEngine);
	mainEngine = false;
	setOrientation(ORIENTATION_0, mainEngine);
	
	//Console at top screen, bottom is 3D + Touch
	bool isDirectFramebuffer = true;
	bool disableTSCWhenTGDSConsoleTop = false;
	bool SaveConsoleContext = false;	//no effect because directFB == true
	u8 * FBSaveContext = NULL;			//no effect because directFB == true
	TGDSLCDSwap(disableTSCWhenTGDSConsoleTop, isDirectFramebuffer, SaveConsoleContext, FBSaveContext);
	
	InitializeConsole(DefaultSessionConsole);
	
	//TGDS Console defaults
	GUI.consoleAtTopScreen = false;	//GUI console at bottom screen
	GUI.consoleBacklightOn = true;	//Backlight On for console
	SWAP_LCDS();
	
	return true;
}

//Definition that overrides the weaksymbol expected from toolchain to init console video subsystem
//1) VRAM Layout
vramSetup * TGDSAUDIOPLAYER_2DVRAM_SETUP(){
	
	vramSetup * vramSetupInst = (vramSetup *)&vramSetupDefaultConsole;
	
	//Main Engine Setup: 
	REG_DISPCNT	=	(uint32)(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D | DISPLAY_SPR_2D_BMP_256);	//Raw "DPG" Mode for main engine
	
	vramSetupInst->vramBankSetupInst[VRAM_A_INDEX].vrambankCR = VRAM_A_0x06000000_ENGINE_A_BG;	//console here
	vramSetupInst->vramBankSetupInst[VRAM_A_INDEX].enabled = true;
	
	vramSetupInst->vramBankSetupInst[VRAM_B_INDEX].vrambankCR = VRAM_B_0x06020000_ENGINE_A_BG;	//Main Engine: ARM9 128K @ 0x06020000	//BackBuffer to allow to transform current 3GP RGB24 Image
	vramSetupInst->vramBankSetupInst[VRAM_B_INDEX].enabled = true;
	
	vramSetupInst->vramBankSetupInst[VRAM_C_INDEX].vrambankCR = VRAM_C_0x06200000_ENGINE_B_BG;	//NDS BMP rgb15 mode + keyboard
	vramSetupInst->vramBankSetupInst[VRAM_C_INDEX].enabled = true;
	
	vramSetupInst->vramBankSetupInst[VRAM_D_INDEX].vrambankCR = VRAM_D_0x06000000_ARM7;	//ARM7 128K @ 0x06000000
	vramSetupInst->vramBankSetupInst[VRAM_D_INDEX].enabled = true;
	
	// 80K for Sprites
	//(SNES:32K -NDSVRAM 64K @ 0x6400000)
	vramSetupInst->vramBankSetupInst[VRAM_E_INDEX].vrambankCR = VRAM_E_0x06400000_ENGINE_A_BG;
	vramSetupInst->vramBankSetupInst[VRAM_E_INDEX].enabled = true;
	
	//(NDSVRAM 16K @ 0x06410000)
	vramSetupInst->vramBankSetupInst[VRAM_F_INDEX].vrambankCR = VRAM_F_0x06410000_ENGINE_A_BG;
	vramSetupInst->vramBankSetupInst[VRAM_F_INDEX].enabled = true;
	
	//vramSetBankG(VRAM_G_BG_EXT_PALETTE);
	vramSetupInst->vramBankSetupInst[VRAM_G_INDEX].vrambankCR = VRAM_G_SLOT01_ENGINE_A_BG_EXTENDED;
	vramSetupInst->vramBankSetupInst[VRAM_G_INDEX].enabled = true;
	
	// 48ko For CPU 
	//vramSetBankH(VRAM_H_LCD);
	vramSetupInst->vramBankSetupInst[VRAM_H_INDEX].vrambankCR = VRAM_H_LCDC_MODE;
	vramSetupInst->vramBankSetupInst[VRAM_H_INDEX].enabled = true;
	
	//vramSetBankI(VRAM_I_LCD);
	vramSetupInst->vramBankSetupInst[VRAM_I_INDEX].vrambankCR = VRAM_I_LCDC_MODE;
	vramSetupInst->vramBankSetupInst[VRAM_I_INDEX].enabled = true;
	
	
	return vramSetupInst;
}