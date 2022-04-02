![ToolchainGenericDS](img/TGDS-Logo.png)

NTR/TWL SDK: TGDS1.65

master: Development branch. Use TGDS1.65: branch for stable features.

This is the ToolchainGenericDS-LinkedModule project:

1.	Compile Toolchain:
To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds :
Then simply extract the project somewhere.

2.	Compile this project: 
Open msys, through msys commands head to the directory your extracted this project.
Then write:
make clean <enter>
make <enter>

After compiling, run the example in NDS. 

Module Specific description:
A ToolchainGenericDS LinkedModule (TGDS-LM) to be used with TGDS apps. Implements ARM9 module loading from the host TGDS Project (SD filesystem), while allowing to return to it.

//How to call the module: 
//Arg0: char * TGDSLinkedModuleFilename = "0:/ToolchainGenericDS-linkedmodule.bin", the module itself. (/release/arm7dldi-ntr/TGDS-lm-template.bin)
//ARGV Support: See ToolchainGenericDS-template/arm9/source/WoopsiTemplate.cpp -> void WoopsiTemplate::handleValueChangeEvent(const GadgetEventArgs& e) method, as it implements ARGV into TGDS-LM.
	//Arg1: Argument Count you want to send over the module 
	//Arg2: Argument Vector you want to send over the module
//Arg3: char * TGDSProjectName. (Inherits the TGDS Host project name from TGDSPROJECTNAME variable)
See ToolchainGenericDS-template's TGDS-LM implementation

/release folder has the latest binary precompiled for your convenience.

Latest stable release: https://bitbucket.org/Coto88/ToolchainGenericDS-linkedmodule/get/TGDS1.65.zip

Notes/Limitations: 
- TGDS ARM9.bin only 
- 2MB max size 
- Implementation entrypoint (main method) as usual is at: arm9/source/main.cpp
- ARM7 code implementation must be done in Host TGDS App as usual. (Host TGDS App: /common /arm7 (ARM7 sided functionality as fifo, sound, etc))
- ITCM/DTCM regions can't be touched. All code runs in the allowed TGDS-LM region.
- Specs: https://bitbucket.org/Coto88/toolchaingenericds-misc/raw/a2e618724f0ba98b09bfa8eb5a0a39efd7a61626/resources/readme.md -> Section "ToolchainGenericDS-LinkedModule"

[Videoplayer module]
Usage:
Use TGDS-helper to generate .TVS videostreams (video + audio track).
Button Start -> A: Choose file

Authors:
IMA-ADPCM Decoder: Discostew

LZSS decompressor: RocketRobz
https://github.com/RocketRobz/RocketVideoPlayer

ToolchainGenericDS SDK, ToolchainGenericDS-videoplayer NTR and TWL ports: Coto + other authors 
https://bitbucket.org/Coto88/toolchaingenericds/src

Changelog:

-0.3:
- TGDS-Videoplayer NTR/TWL port implemented

-0.2:
- add LZSS compression (from the encoder and TGDS-Videoplayer), shrinking .TVS filesizes in about half.
- because of the variable, and way lower filesizes, sound streaming works better now and quality has been increassed to 22Khz by default

-0.1 Alpha:
- Plays up to 10FPS videos (yeah slow I know), but the ARM7DLDI implementation causes some bottlenecks. Audio: IMA-PCM up to 22khz

How to generate .TVS video streams compatible with ToolchainGenericDS-Videoplayer:
https://bitbucket.org/Coto88/toolchaingenericds-helper/src/master/

demo:
misc/fma1.tvs
misc/fma1.ima

Intro of Full Metal Alchemist 1 Japanese intro converted into .TVS format (if the authors want me to take it down I will)

/release folder has the latest binary precompiled for your convenience.

Latest stable release: https://bitbucket.org/Coto88/toolchaingenericds-videoplayer/get/TGDS1.65.zip

Coto