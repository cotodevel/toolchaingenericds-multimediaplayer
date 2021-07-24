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


# relative path (if you are outside arm9 folder then $(DIR_ARM9), arm7 is $(DIR_ARM7))
RELATIVE_PATH =	

#Prepare ARM7 dirs
DIRLIBS_HDR = $(foreach dirres,$(DIRS_ARM7_HEADER),-I "$(CURDIR)/$(RELATIVE_PATH)$(dirres)" )	$(DIRLIBS_ALLOWED)	$(TARGET_LIBRARY_HEADERS_7_INC)
DIRLIBS_SRC = $(foreach dirres,$(DIRS_ARM7_SRC),-I "$(CURDIR)/$(RELATIVE_PATH)$(dirres)" )

file_arm7_src_c=  $(foreach dir,$(DIRS_ARM7_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.c))  )
file_arm7_src_o=  $(foreach dir,$(BUILD), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.o))  )
file_arm7_src_o_out = $(filter-out $(TARGET_LIBRARY_CRT0_FILE_7).o, $(file_arm7_src_o))
file_arm7_src_s=  $(foreach dir,$(DIRS_ARM7_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.s))  )
file_arm7_src_SUPP=  $(foreach dir,$(DIRS_ARM7_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.S))  )
file_arm7_src_cpp=  $(foreach dir,$(DIRS_ARM7_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.cpp))  )
file_arm7_src_bin=  $(foreach dir,$(DIRS_ARM7_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.bin))  )

# Object Target
objs_arm7:=		${file_arm7_src_bin:.bin=.o}	${file_arm7_src_c:.c=.o}	${file_arm7_src_s:.s=.o}	${file_arm7_src_SUPP:.S=.o}		${file_arm7_src_cpp:.cpp=.o}
d_objs_arm7:=	${file_arm7_src_c:.c=.d}	${file_arm7_src_s:.s=.d}	${file_arm7_src_SUPP:.S=.d}	${file_arm7_src_cpp:.cpp=.d}

#ARM7 specific definitions. -fno-common so we force initialized data to rodata
MAPFILE = $(DIR_ARM7).map
AS_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_7)	$(SPECIFIC_AS_COMPILER_FLAGS_7)		$(GLOBAL_AS_COMPILER_FLAGS)
CC_ASFLAGS		=	$(GLOBAL_SHARED_FLAGS_7)	$(SPECIFIC_CC_COMPILER_FLAGS_7)		$(GLOBAL_CC_COMPILER_FLAGS)	-fPIE	-pie
CPP_FLAGS		=	$(GLOBAL_SHARED_FLAGS_7)	$(SPECIFIC_CXX_COMPILER_FLAGS_7)	$(GLOBAL_CXX_COMPILER_FLAGS)	-fPIE	-pie
BUILD			= 	$(BUILD_ARM7)
DIR_SRC			= 	$(DIRS_ARM7_SRC)

#--gc-sections: allows relocatable sections linked (already) to be included, otherwise unused sections are discarded.
#check arm9.map "Discarded input sections" to see --gc-sections in action.
LDFLAGS	:=	-specs=$(TARGET_LIBRARY_LINKER_PATH_LD)/nds_arm7_ld.specs	-Wl,--no-gc-sections,-Map,$(MAPFILE),--fix-v4bx
LDFLAGS_DSI	:=	-specs=$(TARGET_LIBRARY_LINKER_PATH_LD)/dsi_arm7_ld.specs	-Wl,--gc-sections,-Map,arm7-nonstripped_dsi.map,--fix-v4bx
LIBRARIES_NTR =	-l$(TARGET_LIBRARY_TGDS_NTR_7)	-lstdc++	-lcnano7	-lgcc
LIBRARIES_TWL =	-l$(TARGET_LIBRARY_TGDS_TWL_7)	-lstdc++	-lcnano7	-lgcc

################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS:=	$(objs_arm7)	$(file_arm7_src_o_out)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)
OBJPROG =	$(addprefix $(OBJDIR)/, $(NONSTRIPELF_ARM7))

#------------------------------------------ CLANG START
LD2 := $(CC)
CC := clang-8
AR := llvm-ar
DEP_DIR := $(BUILD_DIR)/dep
DEP_CFLAGS := -MP 

# CFLAGS which can provided from the CLI
# They will be appended last and will consequently override
# any pre-existing settings
CLI_CFLAG_OVERRIDES ?=
CLI_LDFLAG_OVERRIDES ?=

# Note: These arguments will be used by clang to select
# the specific architecture target when we use the
# generic --target=arm-none-eabi
ARCH_CFLAGS += \
  -marm -mcpu=arm7tdmi \
  -mfloat-abi=soft	\
  -mfpu=none

COMPILER_SPECIFIC_CFLAGS += \
  --target=arm-none-eabi \
  --sysroot= \
  -Oz	\
  -fno-rtti	-fno-exceptions \
  -DARM7
  
CFLAGS = \
  -MT $@ -MMD \
  -g3 \
  -ffunction-sections -fdata-sections \
  -Wall \
  -Werror \
  -nostdlib \
  -fshort-enums	\
  $(COMPILER_SPECIFIC_CFLAGS) \
  $(ARCH_CFLAGS) \
  $(CLI_CFLAG_OVERRIDES)
	
#------------------------------------------ CLANG END

all: makebuild	$(TARGET_LIBRARY_CRT0_FILE_7).o	$(BINSTRIP_RULE_7)
#all: debug

makebuild:
	-mkdir -p $(OBJDIR)

#rule for reading *.bin in /data folder, converts to .C source code, which then gets compiled as object so it can be linked later
$(OBJDIR)/%.o : %.bin
	-@echo 'trying to convert .BIN ($<) -> .C output: ${@:.o=.c} : arrayName: $(subst build/,,${@:.o=})'
	$(BIN2C)	$<	${@:.o=.c}	$(subst build/,,${@:.o=})
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ ${@:.o=.c}
	
#Do C Compile now!
$(OBJDIR)/%.o : %.c
	-@echo 'C Compile start: $@'
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ $< 
	-@echo 'C Compile End: $@'

#Do C++ (CXX) Compile now!
$(OBJDIR)/%.o : %.cpp
	-@echo 'C++ Compile start: $@'
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ $<
	-@echo 'C Compile End: $@'

#Coto: Special "exception" rule. Fixes tricky projects such as SnemulDS's opcodes2.s where GCC AS is used, while compatibility with .S is required for newer projects.
#.S is GCC CC/CPP for compiling assembler with cpp headers
#.s is GCC AS Assembler for compiling assembler the usual way

#Do s (GCC AS) Compile now!
$(OBJDIR)/%.o : %.s
	-@echo 's Compile start: $@'
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ $< 
	-@echo 's Compile end: $@'

#Do S (GCC CC) Compile now!
$(OBJDIR)/%.o : %.S
	-@echo 'S Compile start: $@'
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $@ $< 
	-@echo 'S Compile end: $@'

#Special rule: compile crt0 for arm7 entrypoint
$(TARGET_LIBRARY_CRT0_FILE_7).o:	$(TARGET_LIBRARY_LINKER_FILE_7)
	-@echo 'S Compile start: crt0s'
	rm -fr $(BUILD)/$(TARGET_LIBRARY_CRT0_FILE_7).o
	$(CC) $(DEP_CFLAGS) $(CFLAGS) $(DIRLIBS_HDR) $(DIRLIBS_SRC)	-c -o $(OBJDIR)/$@	$< 
	cp	-r $(TARGET_LIBRARY_LINKER_PATH_LD)nds_arm7_ld.ld $(CURDIR)
	cp	-r $(TARGET_LIBRARY_LINKER_PATH_LD)nds_arm7_ld.specs $(CURDIR)
	cp	-r $(TARGET_LIBRARY_LINKER_PATH_LD)dsi_arm7.ld $(CURDIR)
	cp	-r $(TARGET_LIBRARY_LINKER_PATH_LD)dsi_arm7_ld.specs $(CURDIR)
	
	-@echo 'S Compile end: crt0s'

#Linkage.
$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJECTS))
	-@echo 'Entering Linking phase'
	-$(LD2) $^	$(LDFLAGS)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_SHARED)	-L	$(TARGET_LIBRARY_PATH)/	-L	$(TOOLCHAIN_PATH)/		$(LIBRARIES_NTR)	-L	$(TARGET_LIBRARY_LINKER_PATH_LD)	-o $@
	-$(LD2) $^	$(LDFLAGS_DSI)	$(DIRLIBS_HDR)	$(DIRLIBS_SRC)	$(DIRLIBS_SHARED)	-L	$(TARGET_LIBRARY_PATH)/	-L	$(TOOLCHAIN_PATH)/		$(LIBRARIES_TWL)	-L	$(TARGET_LIBRARY_LINKER_PATH_LD)	-o	${@:.elf=_dsi.elf}
	-@echo 'Leaving Linking phase. Output: $@'

#xxxx-nonstripped.elf -> xxxx.elf STRIP operation
$(ELF_ARM7): $(OBJPROG)
	-@echo '$@ Unstripped. STRIP Start!'
	-mv build/${@:.elf=-nonstripped_dsi.elf}	$(CURDIR)
	$(OBJSTRIP)	-s -R .comment -R .gnu.version	$<
	-mv	$<	$@
	
$(BINSTRIP_RULE_7): $(ELF_ARM7)
	-@echo 'STRIP End: Remove ELF header and build baremetal Binary executable: $<	->	$@ !'
	$(OBJCOPY) 	-S -O binary	$<	$@
	-rm	$<
################################################################################################
	
clean:
	-@echo 'cleanup $(DIR_ARM7)'
	-@rm -fr $(BINSTRIP_RULE_7)	$(MAPFILE)	$(CURDIR)/nds_arm7_ld.specs	$(CURDIR)/nds_arm7_ld.ld	$(CURDIR)/dsi_arm7_ld.specs	$(CURDIR)/dsi_arm7.ld	$(CURDIR)/arm7-nonstripped_dsi.elf $(CURDIR)/arm7-nonstripped_dsi.map