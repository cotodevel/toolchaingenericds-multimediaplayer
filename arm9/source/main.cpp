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

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"

#include "devoptab_devices.h"
#include "fatfslayerTGDS.h"
#include "usrsettingsTGDS.h"
#include "exceptionTGDS.h"
#include "keypadTGDS.h"
#include "fileHandleTGDS.h"
#include "dswnifi_lib.h"

//C++ part
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iterator>

using namespace std;

char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH+1];

//test1
//default class instance
class cl {
  int i; // private by default
public:
  int get_i();
  void put_i(int j);
};

int cl::get_i()
{
  return i;
}

void cl::put_i(int j)
{
  i = j;
}

//test2
//constructor example
// Class to represent a box
class Box
{
  public:
    int length;
    int breadth;
    int height;

    // Constructor
    Box(int lengthValue, int breadthValue, int heightValue)
    {
      printf("Box constructor called");
      length = lengthValue;
      breadth = breadthValue;
      height = heightValue;
    }

    // Function to calculate the volume of a box
    int volume()
    {
      return length * breadth * height;
    }
};

//test 3
//class copy
class myclass {
  int *p;
public:
  myclass(int i);
  ~myclass();
  int getval() { return *p; }
};

myclass::myclass(int i)
{
  printf("Allocating p");
  p = new int;
  if(!p) {
    printf("Allocation failure.");
    exit(1); // exit program if out of memory
  }
  *p = i;
}

myclass::~myclass()
{
  printf("Freeing p");
  delete p;
}

// when this function is called, the copy constructor is called
void display(myclass ob)
{
	printf("%d",ob.getval());
}


//test4
class myclass2 {
  int *p;
public:
  myclass2(int i);
  ~myclass2();
  int getval() { return *p; }
};

myclass2::myclass2(int i)
{
	printf("Allocating p");
	p = new int;
	if(!p) {
		printf("Allocation failure.");
		exit(1); // exit program if out of memory
	}
	*p = i;
}

// use destructor to free memory
myclass2::~myclass2()
{
  printf("Freeing p");
  delete p;
}

void display2(myclass2 &ob)
{
	printf("%d",ob.getval());
}

string ToStr( char c ) {
   return string( 1, c );
}

std::string getDldiDefaultPath(){
	std::string dldiOut = string((char*)getfatfsPath( (sint8*)string(dldi_tryingInterface() + string(".dldi")).c_str() ));
	return dldiOut;
}


void menuShow(){
	clrscr();
	printf("                              ");
	printf("Start: File Browser -> Press A to play .AAC ");
	printf("B: Stop audio playback ");
	printf("Select: this menu");
}

vector<string> splitCustom(string str, string token){
    vector<string>result;
    while(str.size()){
        int index = str.find(token);
        if(index != (int)string::npos){
            result.push_back(str.substr(0,index));
            str = str.substr(index+token.size());
            if(str.size()==0)result.push_back(str);
        }else{
            result.push_back(str);
            str = "";
        }
    }
    return result;
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
		
		//reload DIR (backward)
		else if(pressed&KEY_B){
			reloadDirB = true;
			break;
		}
		
		else if(pressed&KEY_START){
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
		}
		lastVal = k;
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
	
	if(internalName.at(k)->type == FT_DIR){
		sprintf((char*)curChosenBrowseFile,"%s",internalName.at(k)->fd_namefullPath);
	}
	else{
		sprintf((char*)curChosenBrowseFile,"%s",internalName.at(k)->fd_namefullPath);
	}
	
	clrscr();
	printf("                                   ");
	if(internalName.at(k)->type == FT_DIR){
		printf("you chose Dir:%s",curChosenBrowseFile);
	}
	else{
		printf("you chose File:%s",curChosenBrowseFile);
	}
	return false;
}

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
	
	//custom Handler
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
			
			
			//Audio playback here....
			
			
			
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
			
		}
		
		IRQVBlankWait();
	}
	
	return 0;
}