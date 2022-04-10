![ToolchainGenericDS](img/TGDS-Logo.png)

NTR/TWL SDK: TGDS1.65

master: Development branch. Use TGDS1.65: branch for stable features.

This is the ToolchainGenericDS-videoplayer project:

1.	Compile Toolchain:
To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds :
Then simply extract the project somewhere.

2.	Compile this project: 
Open msys, through msys commands head to the directory your extracted this project.
Then write:
make clean <enter>
make <enter>

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