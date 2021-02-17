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

#define TGDSPROJECTNAME ((char*) "ToolchainGenericDS-audioplayer")

class WoopsiTemplate : public Woopsi, public GadgetEventHandler {
public:
	void startup(int argc, char **argv);
	void shutdown();
	void handleValueChangeEvent(const GadgetEventArgs& e);	//Handles UI events if they change
	void handleClickEvent(const GadgetEventArgs& e);	//Handles UI events when they take click action
	void waitForAOrTouchScreenButtonMessage(MultiLineTextBox* thisLineTextBox, const WoopsiString& thisText);
	void handleLidClosed();
	void handleLidOpen();
	void ApplicationMainLoop();
	FileRequester* _fileReq;
	int currentFileRequesterIndex;
	AmigaScreen* _fileScreen;
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
extern "C" {
#endif

extern WoopsiTemplate * WoopsiTemplateProc;
extern u32 pendPlay;
extern char currentFileChosen[256+1];

#ifdef __cplusplus
}
#endif
