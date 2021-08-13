SDK: TGDS1.6

This is the ToolchainGenericDS-audioplayer project:

Compile Toolchain: To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds : Then simply extract the project somewhere.

Compile this project: Open msys, through msys commands head to the directory your extracted this project. Then write: make clean make

After compiling, run the example in NDS.

Project Specific description: 

TGDS Audio Player, supported formats are:
-	IMA-ADPCM (Intel)/WAV/MP3/AAC/Ogg/FLAC/NSF/SPC/GBS/+ others working.  Streaming mp3, ogg and acc is stripped since it´s 2019 and the DS does not support HTTPS (TLS1.2+)

Button (Start): Reads a file named filelist.txt
Button (X): Builds a list of the current root sd card files/directories into a file named filelist.txt
Button (L): Dump the dldi driver (if exists), into the SD card.
Button (Start): 	Choose a file from a given directory, use D-PAD UP or D-PAD DOWN, D-PAD LEFT or D-PAD RIGHT to navigate directories faster, press A to play: audio files.
Button (B): 		Stop audio playback.
Button (L):      Recent Playlist
Button (D-PAD Down):		Volume Down (-)
Button (D-PAD Up):		Volume Up (+)
Button (Select): This menu screen


/release folder has the latest binary precompiled for your convenience.

Notes:
 
 - DLDI patch the file manually if it isn't detected by the loader.
 - Be careful to NOT to set the volume too high! Prevent hearing loss!

Many thanks to DSOrganize and it's author(s), since the audio player code was taken from there... except there is no memory issues this time to play audio files at all!!!!


Coto
