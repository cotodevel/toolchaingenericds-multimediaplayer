#ifndef _DEMO_H_
#define _DEMO_H_

#ifdef __cplusplus
#include "alert.h"
#include "woopsi.h"
#include "woopsiheaders.h"
#include "filerequester.h"
#include "textbox.h"
#include "soundTGDS.h"
#include "button.h"

#include <string>
using namespace std;

using namespace WoopsiUI;

class WoopsiTemplate : public Woopsi, public GadgetEventHandler {
public:
	void startup(int argc, char **argv);
	void shutdown();
	void handleValueChangeEvent(const GadgetEventArgs& e);	//Handles UI events if they change
	void handleClickEvent(const GadgetEventArgs& e);	//Handles UI events when they take click action
	void waitForAOrTouchScreenButtonMessage(MultiLineTextBox* thisLineTextBox, const WoopsiString& thisText);
	void handleLidClosed();
	void handleLidOpen();
	void ReportAvailableMem();
	void ApplicationMainLoop();
	FileRequester* _fileReq;
	int currentFileRequesterIndex;
	
	AmigaScreen* _controlsScreen;	//Top Screen object
	
	AmigaWindow* _controlWindow;	//Top Screen Controls object
	
	AmigaScreen* _fileScreen;	//Bottom Screen object
	MultiLineTextBox* _MultiLineTextBoxLogger;
	Button* _Index;
	Button* _lastFile;
	Button* _nextFile;
	Button* _play;
	Button* _stop;
	Button* _upVolume;
	Button* _downVolume;
private:
	Alert* _alert;
};
#endif

#endif

#ifdef __cplusplus
extern WoopsiTemplate * WoopsiTemplateProc;
extern "C" {
#endif

extern u32 pendPlay;
extern char currentFileChosen[256+1];
extern void playAudioFile();
extern void stopAudioFile();
extern void resetLayout();
extern void updateLayout();

#ifdef __cplusplus
}
#endif
