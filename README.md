![ToolchainGenericDS](img/TGDS-Logo.png)

NTR/TWL SDK: TGDS1.65

master: Development branch. Use TGDS1.65: branch for stable features.
This is the ToolchainGenericDS-multimediaplayer project:
Compile Toolchain: To compile this project you will need to follow the steps at https://github.com/cotodevel/toolchaingenericds : Then simply extract the project somewhere.
Compile this project: Open msys, through msys commands head to the directory your extracted this project. Then write: make clean make
After compiling, run the example in NDS.

Project Specific description: 

TGDS Multimedia Player, supported formats are:
-	IMA-ADPCM (Intel)/WAV/MP3/ (Up to 192K)AAC - M4A - M4B/Ogg/FLAC/NSF/SPC/GBS/+ others working.  Streaming mp3, ogg and acc is stripped since it�s 2019 and the DS does not support HTTPS (TLS1.2+)
-	.TVS video + audio file support. *.TVS Converter (windows/linux/web) is available at: https://github.com/cotodevel/newlib-nds/blob/TGDS1.65/installer/shared/6.2_2016q4/bin/TVS-SCRIPT-EXPORT-README.MD

NTR Mode Usage:
    - copy all files from /release/arm7dldi-ntr folder to SD:/ root . If it prompts for overwrite: Yes to All. 
    - Run the NDS file
	
TWL Mode Usage:
    - copy all files from /release/arm7dldi-twl folder to SD:/ root . If it prompts for overwrite: Yes to All. 
    - Now open TWiLightMenu (you must set it up first, so you can run TWL mode apps), and run the TWL file. 

Changelog:

-0.4:
- TGDS-Videoplayer implements proper timestamp support. Which means Video + Audio are synchronized properly and it'll never be out of sync again ;-)

-0.3:
- TGDS-Videoplayer NTR/TWL port implemented

-0.2:
- add LZSS compression (from the encoder and TGDS-Videoplayer), shrinking .TVS filesizes in about half.
- because of the variable, and way lower filesizes, sound streaming works better now and quality has been increassed to 22Khz by default

-0.1 Alpha:
- Plays up to 10FPS videos (yeah slow I know), but the ARM7DLDI implementation causes some bottlenecks. Audio: IMA-PCM up to 22khz

How to generate compatible .TVS video streams:
Read above.

demo:
misc/fma1.tvs
misc/fma1.ima

Intro of Full Metal Alchemist 1 Japanese intro converted into .TVS format (if the authors want me to take it down I will)

Buttons:
(L): Previous Audio File 
(R): Next Audio File
(A): Play Audio File 
(B): Stop Audio File 
(D-PAD: Down): Choose file 
(D-PAD: Up): Choose file  
(Select): Toggle Playlist / Repeat mode

Touchscreen: 
Controls the User Interface

Notes:
 - NTR Mode: DLDI patch the file manually if it isn't detected by the loader. TWL Mode doesn't need it.
 - Be careful to NOT to set the volume too high! Prevent hearing loss!
 - AAC/M4A decoder supports up to 32bit files
 - FLAC decoder supports up to 16bit files
 - WAV decoder supports up to 16bit files
 
Authors:
Many thanks to DSOrganize and it's author(s), since the audio player code was taken from there... except there is no memory issues this time to play audio files at all!!!!

IMA-ADPCM Decoder: Discostew

____Remoteboot____
Also, it's recommended to use the remoteboot feature. It allows to send the current TGDS Project over wifi removing the necessity
to take out the SD card repeteadly and thus, causing it to wear out and to break the SD slot of your unit.

Usage:
- Make sure the wifi settings in the NintendoDS are properly set up, so you're already able to connect to internet from it.

- Get a copy of ToolchainGenericDS-multiboot: http://github.com/cotodevel/ToolchainGenericDS-multiboot/archive/TGDS1.65.zip
Follow the instructions there and get either the TWL or NTR version. Make sure you update the computer IP address used to build TGDS Projects, 
in the file: toolchaingenericds-multiboot-config.txt of said repository before moving it into SD card.

For example if you're running NTR mode (say, a DS Lite), you'll need ToolchainGenericDS-multiboot.nds, tgds_multiboot_payload_ntr.binlzss
and toolchaingenericds-multiboot-config.txt (update here, the computer's IP you use to build TGDS Projects) then move all of them to root SD card directory.

- Build the TGDS Project as you'd normally would, and run these commands from the shell.
<make clean>
<make>

- Then if you're on NTR mode:
<remoteboot ntr_mode computer_ip_address>

- Or if you're on TWL mode:
<remoteboot twl_mode computer_ip_address>

- And finally boot ToolchainGenericDS-multiboot, and press (X), wait a few seconds and TGDS Project should boot remotely.
  After that, everytime you want to remoteboot a TGDS Project, repeat the last 2 steps. ;-)


Latest stable release:
http://github.com/cotodevel/toolchaingenericds-multimediaplayer/archive/TGDS1.65.zip


Coto
