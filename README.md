NTR/TWL SDK: TGDS1.64

master: Development branch. Use TGDS1.64Stable: branch for stable features.

This is the ToolchainGenericDS-audioplayer project:

Compile Toolchain: To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds : Then simply extract the project somewhere.

Compile this project: Open msys, through msys commands head to the directory your extracted this project. Then write: make clean make

After compiling, run the example in NDS.

Project Specific description: 

TGDS Audio Player, supported formats are:
-	IMA-ADPCM (Intel)/WAV/MP3/ (Up to 192K)AAC - M4A - M4B/Ogg/FLAC/NSF/SPC/GBS/+ others working.  Streaming mp3, ogg and acc is stripped since it´s 2019 and the DS does not support HTTPS (TLS1.2+)

Buttons:
(Start): File Browser -> (A) to play audio file
(L): Recent Playlist 
(R): Random audio file playback
(B): Stop audio playback 
(X): Mandelbrot demo 
(D-PAD: Down): Volume - 
(D-PAD: Up): Volume + 
(Select): this menu

/release folder has the latest binary precompiled for your convenience.

Latest stable release:
https://bitbucket.org/Coto88/toolchaingenericds-audioplayer/get/TGDS1.64Stable.zip

Notes:
 
 - DLDI patch the file manually if it isn't detected by the loader.
 - Be careful to NOT to set the volume too high! Prevent hearing loss!

Many thanks to DSOrganize and it's author(s), since the audio player code was taken from there... except there is no memory issues this time to play audio files at all!!!!


Coto
