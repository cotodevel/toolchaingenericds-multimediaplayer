![ToolchainGenericDS](img/TGDS-Logo.png)

NTR/TWL SDK: TGDS1.65

master: Development branch. Use TGDS1.65: branch for stable features.

This is the ToolchainGenericDS-multimediaplayer project:

Compile Toolchain: To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds : Then simply extract the project somewhere.

Compile this project: Open msys, through msys commands head to the directory your extracted this project. Then write: make clean make

After compiling, run the example in NDS.

Project Specific description: 

TGDS Multimedia Player, supported formats are:
-	IMA-ADPCM (Intel)/WAV/MP3/ (Up to 192K)AAC - M4A - M4B/Ogg/FLAC/NSF/SPC/GBS/+ others working.  Streaming mp3, ogg and acc is stripped since it´s 2019 and the DS does not support HTTPS (TLS1.2+)
-	.TVS video + audio file support: See https://bitbucket.org/Coto88/toolchaingenericds-multimediaplayer/src/master/ToolchainGenericDS-lm-videoplayer/ for compatible multimedia files conversion 

NTR Mode Usage:
    - copy all files from /release/arm7dldi-ntr folder to SD:/ root . If it prompts for overwrite: Yes to All. 
    - Run the NDS file
	
TWL Mode Usage:
    - copy all files from /release/arm7dldi-twl folder to SD:/ root . If it prompts for overwrite: Yes to All. 
    - Now open TWiLightMenu (you must set it up first, so you can run TWL mode apps), and run the TWL file. 

Buttons:
(Start): File Browser -> (A) to play multimedia file
(L): Recent Playlist 
(R): Random audio file playback
(B): Stop audio playback 
(X): Mandelbrot demo 
(D-PAD: Down): Volume - 
(D-PAD: Up): Volume + 
(Select): this menu

Latest stable release:
https://bitbucket.org/Coto88/toolchaingenericds-multimediaplayer/get/TGDS1.65.zip

Notes:
 - TWL mode todo. Only NTR mode implemented now. 
 - DLDI patch the file manually if it isn't detected by the loader.
 - Be careful to NOT to set the volume too high! Prevent hearing loss!

Many thanks to DSOrganize and it's author(s), since the audio player code was taken from there... except there is no memory issues this time to play audio files at all!!!!


Coto
