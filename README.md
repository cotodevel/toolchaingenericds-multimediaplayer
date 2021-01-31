![ToolchainGenericDS](img/TGDS-Logo.png)


This is the ToolchainGenericDS-audioplayer Woopsi template project:

1.	Compile Toolchain:
To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds :
Then simply extract the project somewhere.

2.	Compile this project: 
Open msys, through msys commands head to the directory your extracted this project.
Then write:
make clean <enter>
make <enter>

After compiling, run the example in NDS. 

Project Specific description:
A Woopsi UI + TGDS SDK template for UI control handling on the NintendoDS. 
Draw windows across the screen, using the stylus.

Also, the homebrew plays these audio formats:
-	IMA-ADPCM (Intel)/WAV/MP3/AAC/Ogg/FLAC/NSF/SPC/GBS/+ others working.  Streaming mp3, ogg and acc is stripped since it´s 2019 and the DS does not support HTTPS (TLS1.2+)

/release folder has the latest binary precompiled for your convenience.
Many thanks to DSOrganize and it's author(s), since the audio player code was taken from there... except there is no memory issues this time to play audio files at all!!!!

Note: UI Controls experimental.

Coto