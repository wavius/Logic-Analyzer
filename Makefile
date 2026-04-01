# ==============================================================================
# Unified Makefile: Auto-detects Windows vs Linux (Fedora)
# ==============================================================================

# --- 1. OS Detection ---
ifeq ($(OS),Windows_NT)
    PLATFORM := WINDOWS
    INSTALL  := C:/intelFPGA_pro/24.1
    SHELL    := cmd.exe
    BASH     := $(INSTALL)/fpgacademy/AMP/cygwin64/bin/bash --noprofile -norc -c
    RM       := /usr/bin/rm -f
    EXE      := .exe
    TOOLCHAIN_BIN := $(INSTALL)/fpgacademy/AMP/cygwin64/home/compiler/bin
    CYG_PATH := export PATH=/usr/local/bin:/usr/bin:$(shell $(BASH) 'cygpath $(INSTALL)')/fpgacademy/AMP/bin
else
    PLATFORM := LINUX
    INSTALL  := /home/wavius/Programs/altera_pro
    SHELL    := /bin/bash
    BASH     :=
    RM       := rm -f
    EXE      :=
    # Path logic for Linux
    TOOLCHAIN_BIN  := $(INSTALL)/riscfree/toolchain/riscv32-unknown-elf/bin
    QUARTUS_BIN    := $(INSTALL)/quartus/bin
    PGM_BIN        := $(INSTALL)/qprogrammer/quartus/bin
    GDB_SERVER_DIR := $(INSTALL)/riscfree/debugger/gdbserver-riscv

    # Linux-specific export to ensure sub-shells find the Altera/Intel binaries
    export PATH := $(QUARTUS_BIN):$(PGM_BIN):$(GDB_SERVER_DIR):$(TOOLCHAIN_BIN):$(INSTALL)/quartus/sopc_builder/bin:$(PATH)
endif

# --- 2. Configuration & Mode ---
MODE := HW
MAIN := core/sw/src/main.c
HDRS := $(wildcard core/sw/inc/*.h)
SRCS := $(wildcard core/sw/src/*.c)
ELF  := $(basename $(MAIN)).elf

# Hardware File Paths (Linux)
HW_DE1-SoC      := "$(INSTALL)/fpgacademy/Computer_Systems/DE1-SoC/DE1-SoC_Computer/niosVg/DE1_SoC_Computer.sof"
JTAG_INDEX_SoC  := 2

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
USERCCFLAGS := -g -O1 -ffunction-sections -fno-inline -gdwarf-2 $(DRIVER_FLAG)

# Linux uses double $$ for the linker symbol, Windows usually prefers single. 
# We'll use the escaped version which is safer across environments for Nios V.
USERLDFLAGS := -Wl,--defsym=__stack_pointer$$=0x4000000 -Wl,--defsym,JTAG_UART_BASE=0xff201000 -lm

ARCH_FLAGS  := -march=rv32im_zicsr -mabi=ilp32
CCFLAGS     := -Wall -c $(USERCCFLAGS) $(ARCH_FLAGS) $(INCLUDES)
LDFLAGS     := $(USERLDFLAGS) $(ARCH_FLAGS)
OBJS        := $(patsubst %.c, %.o, $(SRCS))

# --- 6. Colors ---
RED    := \033[31m
GREEN  := \033[32m
CYAN   := \033[36m
DEF    := \033[0m

# ==============================================================================
# Recipes
# ==============================================================================

all: COMPILE

COMPILE: $(ELF)

$(ELF): $(OBJS)
ifeq ($(PLATFORM),WINDOWS)
	@$(BASH) 'cd "$(CURDIR)"; $(RM) $@'
	@$(BASH) 'printf "$(CYAN)Linking [$(PLATFORM) - $(MODE)] $@$(DEF)\n"'
	@$(BASH) 'cd "$(CURDIR)"; $(CYG_PATH); $(LD) $(LDFLAGS) $(OBJS) -o $@'
else
	@$(RM) $@
	@printf "$(CYAN)Linking [$(PLATFORM) - $(MODE)] $@$(DEF)\n"
	@$(LD) $(LDFLAGS) $^ -o $@
endif

%.o: %.c $(HDRS)
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
	@$(BASH) 'cd "$(CURDIR)"; $(RM) $(ELF) $(OBJS)'
else
	@printf "$(RED)Cleaning build files...$(DEF)\n"
	@$(RM) $(ELF) $(OBJS)
endif

# ==============================================================================
# Hardware / Debug Targets
# ==============================================================================

SYMBOLS:
	@$(NM) -p $(ELF)

OBJDUMP:
	@$(OD) -d -S $(ELF)

DE1-SoC:
	quartus_pgm -c "DE-SoC" -m jtag -o "P;$(HW_DE1-SoC)@$(JTAG_INDEX_SoC)"

TERMINAL:
	nios2-terminal --instance 0

GDB_SERVER:
	ash-riscv-gdb-server --device 02D120DD --gdb-port 2454 --instance 1 --probe-type USB-Blaster-2 --transport-type jtag --auto-detect true

GDB_CLIENT:
	riscv32-unknown-elf-gdb -silent \
		-ex "target remote:2454" \
		-ex "set \$$mstatus=0" \
		-ex "set \$$mtvec=0" \
		-ex "load" \
		-ex "set \$$sp=0x4000000" \
		-ex "set \$$pc=_start" \
		-ex "info reg pc sp" \
		$(ELF)

.PHONY: all COMPILE CLEAN DE1-SoC TERMINAL GDB_SERVER GDB_CLIENT SYMBOLS OBJDUMP