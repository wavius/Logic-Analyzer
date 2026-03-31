# ==============================================================================
# Unified Makefile: Auto-detects Windows vs Linux
# ==============================================================================

# --- 1. OS Detection ---
ifeq ($(OS),Windows_NT)
    PLATFORM := WINDOWS
    INSTALL  := C:/intelFPGA_pro/24.1
    SHELL    := cmd.exe
    # Windows/Cygwin specific BASH wrapper
    BASH     := $(INSTALL)/fpgacademy/AMP/cygwin64/bin/bash --noprofile -norc -c
    RM       := /usr/bin/rm -f
    EXE      := .exe
    # Path logic for Windows
    TOOLCHAIN_BIN := $(INSTALL)/fpgacademy/AMP/cygwin64/home/compiler/bin
    # Cygwin pathing for the recipes
    CYG_PATH := export PATH=/usr/local/bin:/usr/bin:$(shell $(BASH) 'cygpath $(INSTALL)')/fpgacademy/AMP/bin
else
    PLATFORM := LINUX
    INSTALL  := /home/wavius/Programs/altera_pro
    SHELL    := /bin/bash
    BASH     :=
    RM       := rm -f
    EXE      :=
    # Path logic for Linux
    TOOLCHAIN_BIN := $(INSTALL)/riscfree/toolchain/riscv32-unknown-elf/bin
endif

# --- 2. Configuration & Mode ---
# MODE = HW (hardware) or SW (software)
MODE := HW
MAIN := core/sw/src/main.c
HDRS := $(wildcard core/sw/inc/*.h)
SRCS := $(wildcard core/sw/src/*.c)

# --- 3. Toolchain Paths ---
CC := $(TOOLCHAIN_BIN)/riscv32-unknown-elf-gcc$(EXE)
LD := $(CC)
NM := $(TOOLCHAIN_BIN)/riscv32-unknown-elf-nm$(EXE)
OD := $(TOOLCHAIN_BIN)/riscv32-unknown-elf-objdump$(EXE)

# --- 4. Driver Selection Logic ---
ifeq ($(MODE), HW)
    DRIVER_FLAG := -DUSE_HW
else
    DRIVER_FLAG := -DUSE_SW
endif

# --- 5. Flags ---
INCLUDES := -I. -Icore/sw/inc -IComputer_Systems/DE1-SoC/software
USERCCFLAGS := -g -O1 -ffunction-sections -fverbose-asm -fno-inline -gdwarf-2 $(DRIVER_FLAG)
USERLDFLAGS := -Wl,--defsym=__stack_pointer$$=0x4000000 -Wl,--defsym,JTAG_UART_BASE=0xff201000 -lm
ARCHCCFLAGS := -march=rv32im_zicsr -mabi=ilp32
ARCHLDFLAGS := -march=rv32im_zicsr -mabi=ilp32

CCFLAGS := -Wall -c $(USERCCFLAGS) $(ARCHCCFLAGS) $(INCLUDES)
LDFLAGS := $(USERLDFLAGS) $(ARCHLDFLAGS)
OBJS    := $(patsubst %, %.o, $(SRCS))

# --- 6. Colors (Linux style codes work in most modern Windows terminals too) ---
RED    := \033[31m
GREEN  := \033[32m
CYAN   := \033[36m
DEF    := \033[0m

# ==============================================================================
# Recipes
# ==============================================================================

COMPILE: $(basename $(MAIN)).elf

$(basename $(MAIN)).elf: $(OBJS)
ifeq ($(PLATFORM),WINDOWS)
	@$(BASH) 'cd "$(CURDIR)"; $(RM) $@'
	@$(BASH) 'printf "$(CYAN)Linking [$(PLATFORM) - $(MODE)]$(DEF)\n"'
	@$(BASH) 'cd "$(CURDIR)"; $(CYG_PATH); $(LD) $(LDFLAGS) $(OBJS) -o $@'
else
	@$(RM) $@
	@printf "$(CYAN)Linking [$(PLATFORM) - $(MODE)]$(DEF)\n"
	@$(LD) $(LDFLAGS) $(OBJS) -o $@
endif

%.c.o: %.c $(HDRS)
ifeq ($(PLATFORM),WINDOWS)
	@$(BASH) 'cd "$(CURDIR)"; $(RM) $@'
	@$(BASH) 'printf "$(GREEN)Compiling [$<]$(DEF)\n"'
	@$(BASH) 'cd "$(CURDIR)"; $(CYG_PATH); $(CC) $(CCFLAGS) $< -o $@'
else
	@$(RM) $@
	@printf "$(GREEN)Compiling [$<]$(DEF)\n"
	@$(CC) $(CCFLAGS) $< -o $@
endif

CLEAN:
ifeq ($(PLATFORM),WINDOWS)
	@$(BASH) 'printf "$(RED)Cleaning build files...$(DEF)\n"'
	@$(BASH) 'cd "$(CURDIR)"; $(RM) $(basename $(MAIN)).elf $(OBJS) core/sw/src/*.o core/sw/src/*.d'
else
	@printf "$(RED)Cleaning build files...$(DEF)\n"
	@$(RM) $(basename $(MAIN)).elf $(OBJS) core/sw/src/*.o core/sw/src/*.d
endif

.PHONY: COMPILE CLEAN
