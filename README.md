This is the ToolchainGenericDS-audioplayer project:

Compile Toolchain: To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds : Then simply extract the project somewhere.

Compile this project: Open msys, through msys commands head to the directory your extracted this project. Then write: make clean make

After compiling, run the example in NDS.

Project Specific description: 

TGDS Audio Player, supported formats are:
-	WAV/MP3/AAC/Ogg/FLAC/NSF/SPC/GBS/+ others working.  Streaming mp3, ogg and acc is stripped since it´s 2019 and the DS does not support HTTPS (TLS1.2+)

Button (Start): File Browser -> (A) to play audio file
Button (L): Recent Playlist 
Button (R): Random audio file playback
Button (B): Stop audio playback
Button (X): Mandelbrot demo
Button (D-PAD Down):		Volume Down (-)
Button (D-PAD Up):		Volume Up (+)
Button (Select): This menu screen


/release folder has the latest binary precompiled for your convenience.

Notes:
 - Audio files may take a few retries before playback, this is a bug due to interrupts.
 - Be careful to NOT to set the volume too high! Prevent Ear damages!

Many thanks to DSOrganize and it's author(s), since the audio player code was taken from there... except there is no memory issues this time to play audio files at all!!!!


Coto
