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

#include <stdlib.h>
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "fileHandleTGDS.h"
#include "dswnifi_lib.h"
#include "keypadTGDS.h"
#include "misc.h"
#include "sound.h"

//C++ part
using namespace std;
#include <cstdlib>
#include <vector>
#include <iostream>

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
	char fname[256];
	sprintf(fname,"%s",Path);
	int j = 0, k =0;
    
	//OK, use the new CWD and build the playlist
	songLst.clear();
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
		j++;
	}
	
	//actual file lister
	clrscr();
	while(k < j ){
		std::string strDirFileName = string(internalName.at(k)->fd_namefullPath);		
		/*
		if(strlen(getTGDSCurrentWorkingDirectory()) == 1){
			strDirFileName.erase(0,1);	//trim the starting "/"
		}
		*/
		if(internalName.at(k)->type == FT_DIR){
			printfCoords(0, k, "--- %s%s",strDirFileName.c_str(),"<dir>");
		}
		else{
			printfCoords(0, k, "--- %s",strDirFileName.c_str());
		}
		k++;
	}
	
	pressed = 0 ;
	k = 0;
	int lastVal = 0;
	
	bool reloadDirA = false;
	bool reloadDirB = false;
	
	std::string newDir = std::string("");
	
	while(1){
		scanKeys();
		pressed = keysPressed();
		if (pressed&KEY_DOWN && k < (j - 1) ){
			k++;
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysPressed();
			}
		}
		if (pressed&KEY_UP && k != 0) {
			k--;
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysPressed();
			}
		}
		
		//reload DIR (forward)
		if( (pressed&KEY_A) && (internalName.at(k)->type == FT_DIR) ){
			newDir = string(internalName.at(k)->fd_namefullPath);
			reloadDirA = true;
			break;
		}
		
		//file chosen
		else if( (pressed&KEY_A) && (internalName.at(k)->type == FT_FILE) ){
			break;
		}
		
		//reload DIR (backward)
		else if(pressed&KEY_B){
			reloadDirB = true;
			break;
		}
		
		
		// Show cursor
		printfCoords(0, k, "*");
		if(lastVal != k){
			printfCoords(0, lastVal, " ");	//clean old
		}
		while(!(pressed&KEY_DOWN) && !(pressed&KEY_UP) && !(pressed&KEY_START) && !(pressed&KEY_A) && !(pressed&KEY_B)){
			scanKeys();
			pressed = keysPressed();
			updateStreamLoop();
		}
		lastVal = k;
		updateStreamLoop();
	}
	
	//enter a dir
	if(reloadDirA == true){
		enterDir((char*)newDir.c_str());
		return true;
	}
	
	//leave a dir
	if(reloadDirB == true){
		//rewind to preceding dir in TGDSCurrentWorkingDirectory
		leaveDir(getTGDSCurrentWorkingDirectory());
		return true;
	}
	
	sprintf((char*)curChosenBrowseFile,"%s",internalName.at(k)->fd_namefullPath);
	clrscr();
	printf("                                   ");
	if(internalName.at(k)->type == FT_DIR){
		//printf("you chose Dir:%s",curChosenBrowseFile);
	}
	else{
		bool success = loadSound(curChosenBrowseFile);
		while(!success)	
		{
			//getNextSoundInternal(false);
			success = loadSound(curChosenBrowseFile);
		}
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
	
	//let's try playing a WAV
	//loadWavToMemory();
	//loadSound("0:/DSOrganize/startup.wav");
	
	while (1){
		scanKeys();
		
		if (keysPressed() & KEY_START){
			
			//as long you keep using directories ShowBrowser will be true
			char startPath[MAX_TGDSFILENAME_LENGTH+1];
			sprintf(startPath,"%s","/");
			while( ShowBrowser((char *)startPath) == true ){
				//navigating DIRs here...
			}
			
			while(keysPressed() & KEY_START){
				scanKeys();
			}
			menuShow();
		}
		
		if (keysPressed() & KEY_SELECT){
			menuShow();
		}
		
		if (keysPressed() & KEY_B){
			//Audio stop here....
			closeSound();
		}
		
		if (keysPressed() & KEY_R){
			//Play Random song from current folder
			int lstSize = songLst.size();
			if(lstSize > 0){
				closeSound();
				IRQVBlankWait();
				
				//pick one and play
				int randFile = rand() % (lstSize+1);
				strcpy(curChosenBrowseFile, (const char *)songLst.at(randFile).c_str());
				bool success = loadSound((char*)curChosenBrowseFile);
				while(!success)	
				{
					success = loadSound((char*)curChosenBrowseFile);
				}
			}
		}
		
		//Audio playback here....
		updateStreamLoop();
		updateStreamLoop();
		updateStreamLoop();
		updateStreamLoop();
		
		checkEndSound();
		
		IRQVBlankWait();
	}
	
	return 0;
}