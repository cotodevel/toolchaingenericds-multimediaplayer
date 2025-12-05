// Includes
#include "WoopsiTemplate.h"
#include "woopsiheaders.h"
#include "bitmapwrapper.h"
#include "bitmap.h"
#include "graphics.h"
#include "rect.h"
#include "gadgetstyle.h"
#include "fonts/newtopaz.h"
#include "woopsistring.h"
#include "colourpicker.h"
#include "filerequester.h"
#include "soundTGDS.h"
#include "main.h"
#include "posixHandleTGDS.h"
#include "keypadTGDS.h"
#include "ipcfifoTGDSUser.h"
#include "loader.h"
#include "sound.h"
#include "TGDS_threads.h"
#include "TGDSVideo.h"

__attribute__((section(".dtcm")))
WoopsiTemplate * WoopsiTemplateProc = NULL;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::startup(int argc, char **argv) {
	Rect rect;

	/** SuperBitmap preparation **/
	// Create bitmap for superbitmap
	Bitmap* superBitmapBitmap = new Bitmap(164, 191);

	// Get a graphics object from the bitmap so that we can modify it
	Graphics* gfx = superBitmapBitmap->newGraphics();

	// Clean up
	delete gfx;

	// Create screens
	_controlsScreen = new AmigaScreen(TGDSPROJECTNAME, Gadget::GADGET_DRAGGABLE, AmigaScreen::AMIGA_SCREEN_SHOW_DEPTH | AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	woopsiApplication->addGadget(_controlsScreen);
	_controlsScreen->setPermeable(true);

	// Add child windows
	_controlWindow = new AmigaWindow(0, 13, 256, 33, "Controls", Gadget::GADGET_DECORATION, AmigaWindow::AMIGA_WINDOW_SHOW_DEPTH);
	_controlsScreen->addGadget(_controlWindow);

	// Controls
	_controlWindow->getClientRect(rect);

	_Index = new Button(rect.x, rect.y, 41, 16, "Index");	//_Index->disable();
	_Index->setRefcon(2);
	_controlWindow->addGadget(_Index);
	_Index->addGadgetEventHandler(this);
	
	_lastFile = new Button(rect.x + 41, rect.y, 17, 16, "<");
	_lastFile->setRefcon(3);
	_controlWindow->addGadget(_lastFile);
	_lastFile->addGadgetEventHandler(this);
	
	_nextFile = new Button(rect.x + 41 + 17, rect.y, 17, 16, ">");
	_nextFile->setRefcon(4);
	_controlWindow->addGadget(_nextFile);
	_nextFile->addGadgetEventHandler(this);
	
	_play = new Button(rect.x + 41 + 17 + 17, rect.y, 40, 16, "Play");
	_play->setRefcon(5);
	_controlWindow->addGadget(_play);
	_play->addGadgetEventHandler(this);
	
	_stop = new Button(rect.x + 41 + 17 + 17 + 40, rect.y, 40, 16, "Stop");
	_stop->setRefcon(6);
	_controlWindow->addGadget(_stop);
	_stop->addGadgetEventHandler(this);
	
	_upVolume = new Button(rect.x + 41 + 17 + 17 + 40 + 40, rect.y, 40, 16, "Vol.+");
	_upVolume->setRefcon(7);
	_controlWindow->addGadget(_upVolume);
	_upVolume->addGadgetEventHandler(this);
	
	_downVolume = new Button(rect.x + 41 + 17 + 17 + 40 + 40 + 40, rect.y, 40, 16, "Vol.-");
	_downVolume->setRefcon(8);
	_controlWindow->addGadget(_downVolume);
	_downVolume->addGadgetEventHandler(this);
	
	// Add File listing screen
	_fileScreen = new AmigaScreen("File List", Gadget::GADGET_DRAGGABLE, AmigaScreen::AMIGA_SCREEN_SHOW_DEPTH | AmigaScreen::AMIGA_SCREEN_SHOW_FLIP);
	woopsiApplication->addGadget(_fileScreen);
	_fileScreen->setPermeable(true);
	_fileScreen->flipToTopScreen();
	// Add screen background
	_fileScreen->insertGadget(new Gradient(0, SCREEN_TITLE_HEIGHT, 256, 192 - SCREEN_TITLE_HEIGHT, woopsiRGB(0, 31, 0), woopsiRGB(0, 0, 31)));
	
	// Create FileRequester
	_fileReq = new FileRequester(7, 10, 250, 150, "Files", "/", GADGET_DRAGGABLE | GADGET_DOUBLE_CLICKABLE,(WoopsiUI::GadgetStyle *)__null, WoopsiString("/ima/tvs/wav/it/mod/s3m/xm/mp3/mp2/mpa/ogg/aac/m4a/m4b/flac/sid/nsf/spc/sndh/snd/sc68/gbs/vgm"));
	_fileReq->setRefcon(1);
	//Bind OK / Cancel buttons 
	_fileReq->getInternalOKButtonObject()->setRefcon(9);
	_fileReq->getInternalCancelButtonObject()->setRefcon(10);
	_fileReq->addGadgetEventHandler(this);
	_fileScreen->addGadget(_fileReq);
	
	_MultiLineTextBoxLogger = NULL;	//destroyable TextBox
	
	TGDSVideoPlayback = false;
	char * fnameIntro = playIntro();
	enableScreenPowerTimeout();

	//WoopsiSDK Initial state defaults
	ReportAvailableMem();
	
	enableDrawing();	// Ensure Woopsi can now draw itself
	redraw();			// Draw initial state
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::ReportAvailableMem() {
	Rect rect;
	_controlsScreen->getClientRect(rect);
	_MultiLineTextBoxLogger = new MultiLineTextBox(rect.x, rect.y + 60, 200, 70, "DS Hardware status\n...", Gadget::GADGET_DECORATION, 5);	// y + 60 px = move the rectangle vertically from parent obj
	_controlsScreen->addGadget(_MultiLineTextBoxLogger);
	
	_MultiLineTextBoxLogger->removeText(0);
	_MultiLineTextBoxLogger->moveCursorToPosition(0);
	
	//0 = playlist / 1 = repeat
	if(playbackMode == 1){
		_MultiLineTextBoxLogger->appendText("Audio Mode: Repeat File");
	}
	else{
		_MultiLineTextBoxLogger->appendText("Audio Mode: Entire Playlist");
	}
	
	_MultiLineTextBoxLogger->appendText("\n");
	
	char arrBuild[256+1];
	sprintf(arrBuild, "Available heap memory: %d", TGDSARM9MallocFreeMemory());
	_MultiLineTextBoxLogger->appendText(WoopsiString(arrBuild));

	_MultiLineTextBoxLogger->redraw();
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::shutdown() {
	Woopsi::shutdown();
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::waitForAOrTouchScreenButtonMessage(MultiLineTextBox* thisLineTextBox, const WoopsiString& thisText) {
	thisLineTextBox->appendText(thisText);
	scanKeys();
	while((!(keysDown() & KEY_A)) && (!(keysDown() & KEY_TOUCH))){
		scanKeys();
	}
	scanKeys();
	while((keysDown() & KEY_A) && (keysDown() & KEY_TOUCH)){
		scanKeys();
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::handleValueChangeEvent(const GadgetEventArgs& e) {
	// Did a gadget fire this event?
	if (e.getSource() != NULL) {
		FileRequester * freqInst = ((FileRequester *)e.getSource());
		if(freqInst->getInternalOKButtonObject()->hasFocus() == true){
			//button pressed:[OK]
			playAudioFile();
		}
		else if(freqInst->getInternalCancelButtonObject()->hasFocus() == true){
			//button pressed:[Cancel]
			resetLayout();	//Reset layout here
		}
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::handleLidClosed() {
	// Lid has just been closed
	_lidClosed = true;

	// Run lid closed on all gadgets
	s32 i = 0;
	while (i < _gadgets.size()) {
		_gadgets[i]->lidClose();
		i++;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::handleLidOpen() {
	// Lid has just been opened
	_lidClosed = false;

	// Run lid opened on all gadgets
	s32 i = 0;
	while (i < _gadgets.size()) {
		_gadgets[i]->lidOpen();
		i++;
	}
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void WoopsiTemplate::handleClickEvent(const GadgetEventArgs& e) {

	
	switch (e.getSource()->getRefcon()) {
		
		//_Index Event
		case 2:{
			//Get fileRequester size, if > 0, set the first element selected
			FileRequester * freqInst = _fileReq;
			FileListBox* freqListBox = freqInst->getInternalListBoxObject();
			if(freqListBox->getOptionCount() > 0){
				freqListBox->setSelectedIndex(0);
			}
			currentFileRequesterIndex = 0;
		}	
		break;
		
		//_lastFile Event
		case 3:{
			soundPrevTrack();
		}	
		break;
		
		//_nextFile Event
		case 4:{
			soundNextTrack();
		}	
		break;
		
		//_play Event
		case 5:{
			playAudioFile();
		}	
		break;
		
		//_stop Event
		case 6:{
			pendPlay = 2; //stop filestream immediately
			stopAudioFile();
			if( (soundData.sourceFmt == SRC_NONE) && (TGDSVideoPlayback == true) ){
				haltTVSVideoUsermode();
			}
		}	
		break;
		
		//_upVolume Event
		case 7:{
			struct touchPosition touch;
			XYReadScrPosUser(&touch);
			volumeUp(touch.px, touch.py);
			
		}	
		break;
		
		//_downVolume Event
		case 8:{
			struct touchPosition touch;
			XYReadScrPosUser(&touch);
			volumeDown(touch.px, touch.py);
		}	
		break;
		
	}
}

void playAudioFile(){
	if(WoopsiTemplateProc != NULL){

		//if ".." was selected through OK, go back / When B button is pressed
		char isDirTGDSProj[MAX_TGDSFILENAME_LENGTH];
		WoopsiString strObj = WoopsiTemplateProc->_fileReq->getSelectedOption()->getText();
		memset(isDirTGDSProj, 0, sizeof(isDirTGDSProj));
		strObj.copyToCharArray(isDirTGDSProj);

		//Build FileRequester here
		if(strcmpi((char*)&isDirTGDSProj[0], "..") == 0){
			FileRequester * freqInst = WoopsiTemplateProc->_fileReq;
			FileListBox* freqListBox = freqInst->getInternalListBoxObject();
			freqListBox->removeAllOptions();
			freqListBox->readDirectory();
			stopAudioFile();
			pendPlay = 2; //stop filestream immediately & indefinitely
		}
		else{
		
			switch(soundData.sourceFmt){
				case(SRC_GBS):
				case(SRC_NSF):
				case(SRC_SNDH):
				case(SRC_SID):
				{
					FileRequester * freqInst = WoopsiTemplateProc->_fileReq;
					FileListBox* freqListBox = freqInst->getInternalListBoxObject();
					int playListIndex = ((int)freqListBox->getSelectedIndex() - 1);
					
					if(playListIndex >= 0){
						soundSetTrackInPayload(playListIndex); 
						pendPlay = 1; //Play file immediately from payload + Play file from filerequester here if payload was already loaded and play button was pressed.
					}

				}break;

				default:{
					//Play WAV/ADPCM if selected from the FileRequester
					WoopsiString strObj = WoopsiTemplateProc->_fileReq->getSelectedOption()->getText();
					memset(currentFileChosen, 0, sizeof(currentFileChosen));
					strObj.copyToCharArray(currentFileChosen);

					if(FAT_FileExists(currentFileChosen) == FT_FILE){
						pendPlay = 1; //play file immediately & indefinitely
					}
					else{
						pendPlay = 2; //stop filestream immediately & indefinitely
					}
				}break;
			}
			
		}
	}
}

void stopAudioFile(){
	stopAudioStreamUser();
}

void resetLayout(){
	if(WoopsiTemplateProc != NULL){
		//Reset layout here
		WoopsiTemplateProc->_fileScreen->flipToTopScreen();
		WoopsiTemplateProc->_controlsScreen->flipToBottomScreen();
	}
}


//pendPlay status:
//1 = play file immediately (turns into -> 0 = play next file or repeat file indefinitely)
//2 = stop filestream immediately & indefinitely
__attribute__((section(".dtcm")))
u32 pendPlay = 0;

//Called once Woopsi events are ended: TGDS Main Loop
__attribute__((section(".itcm")))
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void Woopsi::ApplicationMainLoop() {
	//Earlier.. main from Woopsi SDK.
	
	//Handle TGDS stuff...
	switch(pendPlay){
		//play 
		case(1):{
			switch(soundData.sourceFmt){

				//play file immediately from payload
				case(SRC_GBS):
				case(SRC_NSF):
				case(SRC_SNDH):
				case(SRC_SID):{
					FileRequester * freqInst = WoopsiTemplateProc->_fileReq;
					FileListBox* freqListBox = freqInst->getInternalListBoxObject();
					int playListIndex = ((int)freqListBox->getSelectedIndex() - 1);
					
					if(playListIndex >= 0){
						soundSetTrackInPayload(playListIndex); 
						
						//stop file immediately & indefinitely (prevent reloading the same payload)
						pendPlay = 2;
					}
				}break;

				//play standalone file immediately
				default:{
					soundLoaded = loadSound((char*)currentFileChosen);
			
					if(soundLoaded == false){
						//stop file immediately & indefinitely
						pendPlay = 2;
					}
					else{
						//play file immediately 
						pendPlay = 0;

						int fileCount = 0;
						char * trackName = NULL;
						if(soundData.sourceFmt == SRC_NSF){
							trackName = getNSFMeta(0);
							fileCount = getNSFTotalTracks();
						}

						else if(soundData.sourceFmt == SRC_GBS){
							trackName = gbsMeta(0);
							fileCount = getGBSTotalTracks();
						}
						
						else if(soundData.sourceFmt == SRC_SID){
							trackName = "Track-";
							fileCount = getSIDTotalTracks();
						}

						else if(soundData.sourceFmt == SRC_SNDH){
							trackName = "Track-";
							fileCount = getSNDHTotalTracks();
						}

						//Build FileRequester here
						switch(soundData.sourceFmt){
							case(SRC_GBS):
							case(SRC_NSF):
							case(SRC_SNDH):
							case(SRC_SID):
							{
								char trackNamePayload[MAX_TGDSFILENAME_LENGTH];
								FileRequester * freqInst = WoopsiTemplateProc->_fileReq;
								freqInst->removeAllOptions();
								freqInst->addOption("..", 0);
								int itemIndex = 1;
								for(itemIndex = 1; itemIndex < (fileCount + 1); itemIndex++){
									sprintf(trackNamePayload, "%s [%d/%d]", trackName, itemIndex, fileCount);
									freqInst->addOption(trackNamePayload, itemIndex);
								}
								
								WoopsiTemplateProc->currentFileRequesterIndex = 1; //force the user through index 0 to abandon the loaded payload 
								FileListBox* freqListBox = freqInst->getInternalListBoxObject();
								freqListBox->setSelectedIndex(WoopsiTemplateProc->currentFileRequesterIndex);
							}break;

							default:{
								//On play, find filename in the listbox, and point to it
								FileRequester * freqInst = WoopsiTemplateProc->_fileReq;
								FileListBox* freqListBox = freqInst->getInternalListBoxObject();
								int foundItemIndex = 0;
								int fileIndex = 0;
								for(fileIndex = 0; fileIndex < freqListBox->getOptionCount(); fileIndex++){
									char curFileDirectoryRead[MAX_TGDSFILENAME_LENGTH];
									memset(curFileDirectoryRead, 0, sizeof(curFileDirectoryRead));
									WoopsiString strObj = freqListBox->getOptionByIndex(fileIndex)->getText();
									strObj.copyToCharArray(curFileDirectoryRead);
									if(strcmpi((char*)&curFileDirectoryRead[0], currentFileChosen) == 0){
										foundItemIndex = fileIndex;
										break;
									}
								}
								WoopsiTemplateProc->currentFileRequesterIndex = foundItemIndex;
								freqListBox->setSelectedIndex(WoopsiTemplateProc->currentFileRequesterIndex);
							}
						}
					}
				}break;
			}

			updateLayout();
			WoopsiTemplateProc->ReportAvailableMem();
		}
		break;

		//stop filestream immediately
		case(2):{
			//pendPlay = 0; //allow B button to halt current played file, enabling this will cause files to be played forcefully, and user needs to control playback.
		}
		break;
	}
	
	if(TGDSVideoPlayback == true){
		TGDSVideoRender();
	}
	handleInput();

	bool waitForVblank = false; //= true; cause stutters during audio playback
	struct task_Context * TGDSThreads = getTGDSThreadSystem();
	int threadsRan = runThreads(TGDSThreads, waitForVblank);
}

void updateLayout(){
	//Reset layout just once, if play was successful
	if(soundData.sourceFmt != SRC_NONE){
		resetLayout();
	}
	else{
		//Is it *.TVS? Play it
		char tmpName[256];
		char ext[256];
		char bootldr[256];
		strcpy(tmpName, currentFileChosen);
		separateExtension(tmpName, ext);
		strlwr(ext);
		if(strncmp(ext,".tvs", 4) == 0){
			playTVSFile(currentFileChosen);
		}
	}
}

//Find first file index from FileRequester object
//Returns: File index (>= 0)
//			-1 if no files in the directory
int getFirstFileIndexFromFileRequester(FileRequester * freqInst){
	FileListBox* freqListBox = freqInst->getInternalListBoxObject();
	int lstSize = freqListBox->getOptionCount();
	int foundItemIndex = -1;
	int fileIndex = 0;
	for(fileIndex = 0; fileIndex < lstSize; fileIndex++){
		char curFileDirectoryRead[MAX_TGDSFILENAME_LENGTH];
		memset(curFileDirectoryRead, 0, sizeof(curFileDirectoryRead));
		WoopsiString strObj = freqListBox->getOptionByIndex(fileIndex)->getText();
		strObj.copyToCharArray(curFileDirectoryRead);
		
		if(FAT_FileExists((char*)&curFileDirectoryRead[0]) == FT_FILE){
			foundItemIndex = fileIndex;
			break;
		}
	}
	return foundItemIndex;
}

//Find last file index from FileRequester object
//Returns: File index (>= 0)
//			-1 if no files in the directory
int getLastFileIndexFromFileRequester(FileRequester * freqInst){
	FileListBox* freqListBox = freqInst->getInternalListBoxObject();
	int lstSize = freqListBox->getOptionCount();
	int foundItemIndex = -1;
	int fileIndex = 0;
	for(fileIndex = (lstSize - 1); fileIndex >= 0; fileIndex--){
		char curFileDirectoryRead[MAX_TGDSFILENAME_LENGTH];
		memset(curFileDirectoryRead, 0, sizeof(curFileDirectoryRead));
		WoopsiString strObj = freqListBox->getOptionByIndex(fileIndex)->getText();
		strObj.copyToCharArray(curFileDirectoryRead);
		
		if(FAT_FileExists((char*)&curFileDirectoryRead[0]) == FT_FILE){
			foundItemIndex = fileIndex;
			break;
		}
	}
	return foundItemIndex;
}