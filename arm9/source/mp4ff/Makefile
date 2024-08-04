#
#			Copyright (C) 2017  Coto
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful, but
#WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
#USA
#



#ToolchainGenericDS specific: Use Makefiles from either TGDS, or custom
export SOURCE_MAKEFILE7 = custom
export SOURCE_MAKEFILE9 = custom


#Shared
ifeq ($(TGDS_ENV),windows)
	include $(DEFAULT_GCC_PATH)/Makefile.basenewlib
else
	export TGDS_ENV := linux
	export DEFAULT_GCC_PATH := /usr/arm-none-eabi/lib/newlib-nano-2.1-nds/6.2_2016q4/
	include $(DEFAULT_GCC_PATH)Makefile.basenewlib
endif

#Custom
export BINSTRIP_RULE_7 =	arm7.bin
export BINSTRIP_RULE_9 =	arm9.bin
export DIR_ARM7 = arm7
export BUILD_ARM7	=	build
export DIR_ARM9 = arm9
export BUILD_ARM9	=	build
export ELF_ARM7 = arm7.elf
export ELF_ARM9 = arm9.elf
#NTR TGDS7 & TGDS9
export TARGET_LIBRARY_NAME_7 = 
export TARGET_LIBRARY_NAME_9 = mp4ff
export TARGET_LIBRARY_FILE_7	=	lib$(TARGET_LIBRARY_NAME_7).a
export TARGET_LIBRARY_FILE_9	=	lib$(TARGET_LIBRARY_NAME_9).a
export TARGET_LIBRARY_LIBNAME_7	=	l$(TARGET_LIBRARY_NAME_7)
export TARGET_LIBRARY_LIBNAME_9	=	l$(TARGET_LIBRARY_NAME_9)

.SUFFIXES: .all
#####################################################ARM9#####################################################

export DIRS_ARM9_SRCLIB = data/	\
			build/	\
			/	\
			src/	\
			../tremor/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/
			
export DIRS_ARM9_HEADERLIB = ../../../templateCode/data/arm9/	\
			../../../templateCode/source/	\
			data/	\
			build/	\
			/	\
			src/	\
			../	\
			../aac/	\
			../aac/pub/	\
			../aac/real/	\
			../aac/real/asm/	\
			../libffmpegFLAC/	\
			../libffmpegFLAC/src/	\
			../mp4ff/	\
			../mad/	\
			../mikmod/	\
			../mikmod/drivers/	\
			../mikmod/include/	\
			../mikmod/loaders/	\
			../mikmod/mmio/	\
			../mikmod/playercode/	\
			../tremor/	\
			../tremor/	\
			$(TARGET_LIBRARY_PATH)$(TARGET_LIBRARY_MAKEFILES_SRC)/templateCode/

#######################################################################################

# relative path (if you are outside arm9 folder then $(DIR_ARM9), arm7 is $(DIR_ARM7))
RELATIVE_PATH =	

#Prepare ARM9 dirs
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM9_HEADERLIB),-I "$(RELATIVE_PATH)$(dirres)" )	$(DIRLIBS_ALLOWED)	$(TARGET_LIBRARY_HEADERS_9_INC)	
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM9_SRCLIB),-I "/$(dirres)" )

dir_arm7_src_h=  $(foreach dir,$(DIRS_ARM9_HEADERLIB), $(wildcard $(RELATIVE_PATH)$(dir)*.h))
file_arm9_src_c=  $(foreach dir,$(DIRS_ARM9_SRCLIB), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm9_src_s=  $(foreach dir,$(DIRS_ARM9_SRCLIB), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm9_src_SUPP=  $(foreach dir,$(DIRS_ARM9_SRCLIB), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm9_src_cpp=  $(foreach dir,$(DIRS_ARM9_SRCLIB), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )
file_arm9_src_bin=  $(foreach dir,$(DIRS_ARM9_SRCLIB), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.bin))  )
file_arm9_src_tga=  $(foreach dir,$(DIRS_ARM9_SRCLIB), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.tga))  )
file_arm9_src_pcx=  $(foreach dir,$(DIRS_ARM9_SRCLIB), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.pcx))  )

# Object Target
objs_arm9:=	${file_arm9_src_pcx:.pcx=.o}	${file_arm9_src_tga:.tga=.o}	${file_arm9_src_bin:.bin=.o}	${file_arm9_src_c:.c=.o}	${file_arm9_src_s:.s=.o}	${file_arm9_src_SUPP:.S=.o}		${file_arm9_src_cpp:.cpp=.o}
d_objs_arm9:=	${file_arm9_src_c:.c=.d}	${file_arm9_src_s:.s=.d}	${file_arm9_src_SUPP:.S=.d}	${file_arm9_src_cpp:.cpp=.d}

#ARM9 specific definitions. -fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM9).map
AS_ASFLAGS		=	$(SHARED_FLAGS)	-defsym debug=0	-defsym	appver='$(EXECUTABLE_VERSION)'	-defsym	ARM9='1'
CC_FLAGS		=	$(SHARED_FLAGS)	-Wall	-DNDS -DARM9	-marm	-Os -fno-exceptions	-nostdlib	-fno-common	-MMD -MP	-nostdinc	-Dappver='$(EXECUTABLE_VERSION)'
CPP_FLAGS		=	$(SHARED_FLAGS)	$(CC_FLAGS)	-nostdinc++	-fno-rtti
BUILD			= 	$(BUILD_ARM9)
DIR_SRC			=	$(DIRS_ARM9_SRCLIB)
################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm9)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG = 	$(addprefix $(OBJDIR)/, $(TARGET_LIBRARY_FILE_9))

all: $(OBJDIR)|forceDelete	$(OBJPROG)
#all: debug

forceDelete:
	-@rm -fr ./$(BUILD)/$(TARGET_LIBRARY_FILE_9)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)	

#Check $(OBJDIR) if not exists: create it
$(OBJDIR):
	@echo "Folder $(OBJDIR) does not exist. Creating"
	mkdir -p $@

#rule for reading *.bin in /data folder, converts to .C source code, which then gets compiled as object so it can be linked later
$(OBJDIR)/%.o : %.bin
	-@echo 'trying to convert .BIN ($<) -> .C output: ${@:.o=.c} : arrayName: $(subst build/,,${@:.o=})'
	$(BIN2C)	$<	${@:.o=.c}	$(subst build/,,${@:.o=})
	-mkdir -p $(TARGET_LIBRARY_HEADERS_9)	&&	cp ${@:.o=.h} $(TARGET_LIBRARY_HEADERS_9)
	$(CC)	$(CC_FLAGS)	-std=c99	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_PROHIBITED)	-o $@ -c ${@:.o=.c}

#convert a TGA BMP16 into a native 3D texture unit format (GL_COLOR 16bpp)
$(OBJDIR)/%.o : %.tga
	-$(GRITTOOL) ${<:.o=.c} -ftc -o $@ -gu32 -gb -gB16
	$(CC)	$(CC_FLAGS)	-std=c99	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_PROHIBITED)	-o $@ -c ${@:.o=.c}

$(OBJDIR)/%.o : %.pcx	
	-@echo 'trying to convert .BIN ($<) -> .C output: ${@:.o=.c} : arrayName: $(subst build/,,${@:.o=})'
	$(BIN2C)	$<	${@:.o=.c}	$(subst build/,,${@:.o=})
	-mkdir -p $(TARGET_LIBRARY_HEADERS_9)	&&	cp ${@:.o=.h} $(TARGET_LIBRARY_HEADERS_9)
	$(CC)	$(CC_FLAGS)	-std=c99	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_PROHIBITED)	-o $@ -c ${@:.o=.c}

#Do C Compile now!
$(OBJDIR)/%.o : %.c
	-@echo 'C Compile start: $@'
	$(CC)	$(CC_FLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_PROHIBITED)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Do C++ (CXX) Compile now!
$(OBJDIR)/%.o : %.cpp
	-@echo 'C++ Compile start: $@'
	$(CPP)	$(CPP_FLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	-o $@ -c $<
	-@echo 'C Compile End: $@'

#Coto: Special "exception" rule. Assembler (.s) support ie: .ifdef/.endif (which differs from CPP-like syntax Assembler: ie: #ifdef/#endif by .S)
#.S is GCC CC/CPP for compiling assembler with cpp headers
#.s is GCC AS Assembler for compiling assembler the usual way

#Do s (GCC AS) Compile now!
$(OBJDIR)/%.o : %.s
	-@echo 's Compile start: $@'
	$(AS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(AS_ASFLAGS)	-o $@ -c $<
	-@echo 's Compile end: $@'

#Do S (GCC CC) Compile now!
$(OBJDIR)/%.o : %.S
	-@echo 'S Compile start: $@'
	$(CPP)	-x assembler-with-cpp	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(CC_FLAGS)	-o $@ -c $<
	-@echo 'S Compile end: $@'

#Builder.
$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJECTS))
	-@echo 'Entering Linking phase'
	$(AR) rvs	$@	$^
	mv	$@	$(CURDIR)/../libs/
	-@echo 'Leaving Linking phase. $(OBJPROG)'
	
################################################################################################
clean:
	-@echo 'cleanup $(TARGET_LIBRARY_FILE_9)'
ifeq ($(TGDS_ENV),windows)
	-@rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_9)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)	$(CURDIR)/../libs/$(TARGET_LIBRARY_FILE_9)
endif
ifeq ($(TGDS_ENV),linux)
	-@sudo rm -fr $(OBJDIR)	$(TARGET_LIBRARY_FILE_9)	$(MAPFILE)	$(TARGET_LIBRARY_PATH)/$(TARGET_LIBRARY_FILE_9)	$(CURDIR)/../libs/$(TARGET_LIBRARY_FILE_9)
endif
