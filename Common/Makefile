include param.mk

COMMON_PATH  = ../../Common

COMMON_OBJS  = $(COMMON_PATH)/prg_bt.o $(COMMON_PATH)/prg_bt_addr.o
COMMON_LIB   = $(COMMON_PATH)/libcommon.a
STARTUP_OBJS = $(COMMON_PATH)/prg_disp.o
PROJ_OBJS    = $(STARTUP_OBJS) $(OBJS)

BIN_PATH     = ../../Bin
GCC_PATH     = $(BIN_PATH)/GNU/GNU_ARM
BIN_GCC_PATH = $(GCC_PATH)/bin
LIB_GCC_PATH = $(GCC_PATH)/lib/gcc/arm-elf
LIB_PATH     = $(GCC_PATH)/arm-elf/lib

LD_SCRIPT    = $(COMMON_PATH)/ld.lcf

TARGET_BIN   = $(PROJ).bin
PROJ_ELF     = $(PROJ).elf
PROJ_MAP     = $(PROJ).map
    
CROSS_COMPILE       = arm-elf
CROSS_COMPILE_PRE   = $(CROSS_COMPILE)-
BIN_GCC_PATH_CROSS  = $(CROSS_COMPILE_PRE)

CC      = $(BIN_GCC_PATH_CROSS)gcc
AS_CPP  = $(BIN_GCC_PATH_CROSS)gcc
AS      = $(BIN_GCC_PATH_CROSS)as
AR      = $(BIN_GCC_PATH_CROSS)ar
LD      = $(BIN_GCC_PATH_CROSS)ld
OC      = $(BIN_GCC_PATH_CROSS)objcopy

RM  = $(BIN_PATH)/GNU/Tools/rm

O_LANG    := -x c

GNU_ARM_M_ARCH        :=  -mcpu=arm9e
GNU_ARM_L_ARCH        :=  --architecture=armv5tej

O_OPTIM := -O3

C_INCL                := . $(COMMON_PATH)

P_DEFS_GLOBAL         :=  -DENDIAN_LITTLE

O_GDB                 := -gdwarf-2 -fno-dwarf2-cfi-asm

M_FLAGS_GLOBAL        := $(GNU_ARM_M_ARCH)
__FLAGS_GLOBAL        := $(M_FLAGS_GLOBAL) $(P_DEFS_GLOBAL) $(O_OPTIM) $(O_GDB)

C_FLAGS_GLOBAL        := -S $(__FLAGS_GLOBAL) -fno-builtin $(O_LANG) -Wall

AS_CPP_FLAGS_GLOBAL   := -c $(__FLAGS_GLOBAL) -Wa,-EL -x assembler-with-cpp -D __ASSEMBLY__ -Wall -Wa

A_FLAGS_GLOBAL        := -EL

A_LST                 := -almns=

ARFLAGS               := -rcuv

CFLAGS = \
        -c \
        $(addprefix -I,$(C_INCL))

LFLAGS = \
        --cref \
        --oformat elf32-littlearm \
        --trace \
        --nmagic \
        $(GNU_ARM_L_ARCH)

S_INC  = $(addprefix -I,$(C_INCL))

A_LST  = -almns=

%.o : %.c
	$(CC) $(C_FLAGS_GLOBAL) $(CFLAGS) -C -E $< > $(basename $@).p
	$(CC) $(C_FLAGS_GLOBAL) $(CFLAGS) -o $(basename $@).asm $<
	$(AS) $(A_FLAGS_GLOBAL) $(A_LST)$(basename $@).lst -o $@ $(basename $@).asm

%.o : %.S
	$(AS_CPP) $(AS_CPP_FLAGS_GLOBAL) $(S_INC) -o $(basename $@).o -Wa,$(A_LST)$(basename $@).lst $<

$(COMMON_LIB): $(COMMON_OBJS)
	$(AR) $(ARFLAGS) $(COMMON_LIB) $(COMMON_OBJS)

$(PROJ_ELF): $(PROJ_OBJS) $(LD_SCRIPT)
	$(LD) $(LFLAGS) \
	--library-path=$(COMMON_PATH) \
	--library-path=$(LIB_PATH) \
	--library-path=$(LIB_GCC_PATH) \
	--script=$(LD_SCRIPT) \
	$(PROJ_OBJS) \
	--library=common \
	--library=m \
	--library=c \
	--library=gcc \
	-Map $(PROJ_MAP) \
	-o $(PROJ_ELF)

$(TARGET_BIN): $(COMMON_LIB) $(PROJ_ELF)
	$(OC) --output-target=binary $(PROJ_ELF) $(TARGET_BIN)

.PHONY: all
all: $(TARGET_BIN)

clean:
	$(RM) -f \
		$(PROJ_OBJS) \
		$(PROJ_OBJS:.o=.lst) \
		$(PROJ_OBJS:.o=.p) \
		$(PROJ_OBJS:.o=.asm) \
		$(COMMON_OBJS) \
		$(COMMON_OBJS:.o=.lst) \
		$(COMMON_OBJS:.o=.p) \
		$(COMMON_OBJS:.o=.asm) \
		$(COMMON_LIB) \
		$(PROJ_MAP) \
		$(PROJ_ELF) \
		$(TARGET_BIN)
