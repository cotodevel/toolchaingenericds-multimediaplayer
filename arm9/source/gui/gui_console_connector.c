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
	//bool isTGDSCustomConsole = true;
	//GUI_init(isTGDSCustomConsole);


////////[For default Console implementation simply call]:////////
	//bool isTGDSCustomConsole = false;
	//GUI_init(isTGDSCustomConsole);


	////////[Default Console implementation is selected, thus stubs are implemented here]////////


//Definition that overrides the weaksymbol expected from toolchain to init console video subsystem
ConsoleInstance * getProjectSpecificVRAMSetup(){
	return NULL;
}


//2) Uses subEngine: VRAM Layout -> Console Setup
bool InitProjectSpecificConsole(ConsoleInstance * ConsoleInstanceInst){
	UpdateConsoleSettings(ConsoleInstanceInst);
	return true;
}