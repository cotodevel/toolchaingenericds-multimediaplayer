//Shared Console debugging implementation for ARM7/ARM9 in TGDS environment

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "_console.h"
#include "posixHandleTGDS.h"
#include "InterruptsARMCores_h.h"

#ifdef ARM9
#include "consoleTGDS.h"
#endif

#ifdef ARM7

void _consolePrint(char *chr, int argvCount, int * argv){
	//printf7((char *)chr, argvCount, argv);
}

#endif

#ifdef ARM9
void _consolePrint(const char* s){
	printf("%s", s);
}

//same Implementation as TGDS printf ARM9 (posixHandleTGDS.c)
void _consolePrintf(const char* fmt, ...){
	//Indentical Implementation as GUI_printf
	va_list args;
	va_start (args, fmt);
	vsnprintf ((sint8*)ConsolePrintfBuf, (int)sizeof(ConsolePrintfBuf), fmt, args);
	va_end (args);
	
    // FIXME
    bool readAndBlendFromVRAM = false;	//we discard current vram characters here so if we step over the same character in VRAM (through printfCoords), it is discarded.
	t_GUIZone zone;
    zone.x1 = 0; zone.y1 = 0; zone.x2 = 256; zone.y2 = 192;
    zone.font = &smallfont_7_font;
	
	int color = (int)TGDSPrintfColor_LightGrey;	//default color
	int stringSize = (int)strlen(ConsolePrintfBuf);
	
	//Separate the TGDS Console font color if exists
	char cpyBuf[256+1] = {0};
	strcpy(cpyBuf, ConsolePrintfBuf);
	char * outBuf = (char *)TGDSARM9Malloc(256*10);
	char * colorChar = (char*)((char*)outBuf + (1*256));
	int matchCount = str_split((char*)cpyBuf, ">", outBuf, 10, 256);
	if(matchCount > 0){
		color = atoi(colorChar);
		ConsolePrintfBuf[strlen(ConsolePrintfBuf) - (strlen(colorChar)+1) ] = '\0';
	}
	
    GUI_drawText(&zone, 0, GUI.printfy, color, (sint8*)ConsolePrintfBuf, readAndBlendFromVRAM);
    GUI.printfy += GUI_getFontHeight(&zone);
	TGDSARM9Free(outBuf);
	strlen(ConsolePrintfBuf)+1;
}
#endif

void ShowLogHalt(){
	#ifdef ARM7
	int argBuffer[MAXPRINT7ARGVCOUNT];
	memset((unsigned char *)&argBuffer[0], 0, sizeof(argBuffer));
	_consolePrint("ARM7: ShowLogHalt().", 0, (int*)&argBuffer[0]);
	while(1==1){
		IRQVBlankWait();
	}
	#endif
	
	#ifdef ARM9
	printf("ARM9: ShowLogHalt(). Halting process. ");
	while(1==1){
		IRQVBlankWait();
	}
	#endif
}



void _consoleLogPause(void){
	
}

void _consoleLogResume(void){
	
}
