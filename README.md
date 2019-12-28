This is the ToolchainGenericDS-audioplayer project:

Compile Toolchain: To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds : Then simply extract the project somewhere.

Compile this project: Open msys, through msys commands head to the directory your extracted this project. Then write: make clean make

After compiling, run the example in NDS.

Project Specific description: 

TGDS Audio Player, supported formats are:
-	WAV/MP3/AAC/Ogg/FLAC/NSF/SPC/GBS/+ others working.  Streaming mp3, ogg and acc is stripped since it´s 2019 and the DS does not support HTTPS (TLS1.2+)

Set up:
1)
DLDI patch the ToolchainGenericDS-audioplayer.nds in /release folder (NTR mode only)

2)
Start: 	Choose a file from a given directory, use D-PAD UP or D-PAD DOWN to scroll up/down files in the directory, 
		or D-PAD LEFT or D-PAD RIGHT to navigate directories faster, press A to play: audio files.

B: 		
		Stop audio playback.

L:      Recent Playlist

R:		Audio playback of any random audio file from the current folder

Select: 
		This menu screen

/release folder has the latest binary precompiled for your convenience.

Notes:
- mp3 may take a few retries before playback, this is a bug due to interrupts
- Many thanks to DSOrganize and it's author(s), since the audio stream code was taken from there... 
  except there is no memory issues this time to play audio files at all!!!!


Coto
