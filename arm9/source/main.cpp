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

#include "main.h"

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "gui_console_connector.h"
#include "misc.h"
#include "sound.h"
#include "dswnifi_lib.h"

//C++ part
using namespace std;
#include <string>
#include <vector>

static vector<string> songLst;
char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH+1];

string ToStr( char c ) {
   return string( 1, c );
}

void menuShow(){
	clrscr();
	printf("                              ");
	printf("Supported Formats: WAV/MP3/AAC/Ogg");
	printf("/FLAC/NSF/SPC/GBS/+ others");
	printf("                              ");
	printf("(Start): File Browser -> (A) to play audio file");
	printf("(R): Random audio file playback ");
	printf("(B): Stop audio playback ");
	printf("(Select): this menu");
	if(soundLoaded == false){
		printf("Playback: Stopped.");
	}
	else{
		printf("Playing: %s", curChosenBrowseFile);
	}
}


std::string parseDirNameTGDS(std::string dirName){
	if ((dirName.at(0) == '/') && (dirName.at(1) == '/')) {
		dirName.erase(0,1);	//trim the starting / if it has one
	}
	dirName.erase(dirName.length());	//trim the leading "/"
	return dirName;
}

std::string parsefileNameTGDS(std::string fileName){
	if ((fileName.at(2) == '/') && (fileName.at(3) == '/')) {
		fileName.erase(2,2);	//trim the starting // if it has one (since getfspath appends 0:/)
		if(fileName.at(2) != '/'){	//if we trimmed by accident the only leading / such as 0:filename instead of 0:/filename, restore it so it becomes the latter
			fileName.insert(2, ToStr('/') );
		}
	}
	return fileName;
}

bool ShowBrowser(char * Path){
	while((keysPressed() & KEY_START) || (keysPressed() & KEY_A) || (keysPressed() & KEY_B)){
		scanKeys();
	}
	int pressed = 0;
	vector<struct FileClass *> internalName;
	struct FileClass filStub;
	char fname[256];
	sprintf(fname,"%s",Path);
	int j = 1;
    
	//OK, use the new CWD and build the playlist
	songLst.clear();
	songLst.push_back("stub");
	internalName.push_back(&filStub);
	
	int retf = FAT_FindFirstFile(fname);
	while(retf != FT_NONE){
		struct FileClass * fileClassInst = NULL;
		//directory?
		if(retf == FT_DIR){
			fileClassInst = getFileClassFromList(LastDirEntry);
			std::string outDirName = string(fileClassInst->fd_namefullPath);
			sprintf(fileClassInst->fd_namefullPath,"%s",parseDirNameTGDS(outDirName).c_str());
		}
		//file?
		else if(retf == FT_FILE){
			fileClassInst = getFileClassFromList(LastFileEntry); 
			std::string outFileName = string(fileClassInst->fd_namefullPath);
			
			songLst.push_back(outFileName);
			
			sprintf(fileClassInst->fd_namefullPath,"%s",parsefileNameTGDS(outFileName).c_str());
		}
		internalName.push_back(fileClassInst);
		
		//more file/dir objects?
		retf = FAT_FindNextFile(fname);
	}
	
	//actual file lister
	clrscr();
	
	j = 1;
	pressed = 0 ;
	int lastVal = 0;
	bool reloadDirA = false;
	bool reloadDirB = false;
	std::string newDir = std::string("");
	
	#define itemsShown (int)(15)
	int curjoffset = 0;
	int itemRead=1;
	
	while(1){
		
		int itemsToLoad = (internalName.size() - curjoffset);
		
		//check if remaining items are enough
		if(itemsToLoad > itemsShown){
			itemsToLoad = itemsShown;
		}
		
		while(itemRead < itemsToLoad ){
			std::string strDirFileName = string(internalName.at(itemRead+curjoffset)->fd_namefullPath);		
			if(internalName.at(itemRead+curjoffset)->type == FT_DIR){
				printfCoords(0, itemRead, "--- %s%s",strDirFileName.c_str(),"<dir>");
			}
			else{
				printfCoords(0, itemRead, "--- %s",strDirFileName.c_str());
			}
			itemRead++;
		}
		
		scanKeys();
		pressed = keysPressed();
		if (pressed&KEY_DOWN && (j < (itemsToLoad - 1) ) ){
			j++;
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysPressed();
			}
		}
		
		//downwards: means we need to reload new screen
		else if(pressed&KEY_DOWN && (j >= (itemsToLoad - 1) ) && ((internalName.size() - curjoffset - itemRead) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysPressed();
			}
		}
		
		if (pressed&KEY_UP && (j > 1)) {
			j--;
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysPressed();
			}
		}
		
		//upwards: means we need to reload new screen
		else if (pressed&KEY_UP && (j <= 1) && (curjoffset > 0) ) {
			//list only the remaining items
			clrscr();
			
			curjoffset--;
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysPressed();
			}
		}
		
		
		//reload DIR (forward)
		if( (pressed&KEY_A) && (internalName.at(j+curjoffset)->type == FT_DIR) ){
			newDir = string(internalName.at(j+curjoffset)->fd_namefullPath);
			reloadDirA = true;
			break;
		}
		
		//file chosen
		else if( (pressed&KEY_A) && (internalName.at(j+curjoffset)->type == FT_FILE) ){
			break;
		}
		
		//reload DIR (backward)
		else if(pressed&KEY_B){
			reloadDirB = true;
			break;
		}
		
		// Show cursor
		printfCoords(0, j, "*");
		if(lastVal != j){
			printfCoords(0, lastVal, " ");	//clean old
		}
		lastVal = j;
		updateStreamLoop();
	}
	
	//enter a dir
	if(reloadDirA == true){
		internalName.clear();
		enterDir((char*)newDir.c_str());
		return true;
	}
	
	//leave a dir
	if(reloadDirB == true){
		//rewind to preceding dir in TGDSCurrentWorkingDirectory
		leaveDir(getTGDSCurrentWorkingDirectory());
		return true;
	}
	
	sprintf((char*)curChosenBrowseFile,"%s",internalName.at(j+curjoffset)->fd_namefullPath);
	clrscr();
	printf("                                   ");
	if(internalName.at(j+curjoffset)->type == FT_DIR){
		//printf("you chose Dir:%s",curChosenBrowseFile);
	}
	else{
		soundLoaded = loadSound(curChosenBrowseFile);
	}
	return false;
}

__attribute__((section(".itcm")))
int main(int _argc, sint8 **_argv) {
	
	/*			TGDS 1.5 Standard ARM9 Init code start	*/
	bool project_specific_console = false;	//set default console or custom console: default console
	GUI_init(project_specific_console);
	GUI_clear();
	
	sint32 fwlanguage = (sint32)getLanguage();
	
	printf("     ");
	printf("     ");
	
	int ret=FS_init();
	if (ret == 0)
	{
		printf("FS Init ok.");
	}
	else if(ret == -1)
	{
		printf("FS Init error.");
	}
	switch_dswnifi_mode(dswifi_idlemode);
	/*			TGDS 1.5 Standard ARM9 Init code end	*/
	
	//Init sound
	enableVBlank(); // initialize vblank irq
	setGenericSound(11025, 127, 64, 1);
	initComplexSound(); // initialize sound variables
	
	menuShow();
	
	while (1){
		scanKeys();
		
		if (keysPressed() & KEY_START){
			
			//as long you keep using directories ShowBrowser will be true
			char startPath[MAX_TGDSFILENAME_LENGTH+1];
			sprintf(startPath,"%s","/");
			while( ShowBrowser((char *)startPath) == true ){
				//navigating DIRs here...
			}
			
			scanKeys();
			while(keysPressed() & KEY_START){
				scanKeys();
			}
			menuShow();
		}
		
		if (keysPressed() & KEY_B){
			//Audio stop here....
			closeSound();
			
			menuShow();
			
			scanKeys();
			while(keysPressed() & KEY_B){
				scanKeys();
			}
		}
		
		if (keysPressed() & KEY_R){
			//Play Random song from current folder
			int lstSize = songLst.size();
			if(lstSize > 1){
				closeSound();
				
				//pick one and play
				int randFile = rand() % (lstSize+1);
				strcpy(curChosenBrowseFile, (const char *)songLst.at(randFile).c_str());
				soundLoaded = loadSound((char*)curChosenBrowseFile);
				menuShow();
				
				scanKeys();
				while(keysPressed() & KEY_R){
					scanKeys();
				}
			
			}
		}
		
		//Audio playback here....
		updateStreamLoop();
		updateStreamLoop();
		updateStreamLoop();
		updateStreamLoop();
		IRQVBlankWait();
	}
	
	return 0;
}