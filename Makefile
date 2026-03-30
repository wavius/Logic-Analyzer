############################################
# Paths & Environment
############################################
# Using the path from your previous Linux-style Makefile
INSTALL      := /home/wavius/Programs/altera_pro

# Specific toolchain sub-path
RISCV_BIN    := $(INSTALL)/riscfree/toolchain/riscv32-unknown-elf/bin
GDB_S_BIN    := $(INSTALL)/riscfree/debugger/gdbserver-riscv
QP_BIN       := $(INSTALL)/qprogrammer/quartus/bin
AMP_BIN      := $(INSTALL)/fpgacademy/AMP/bin

# Exporting these for any sub-shells (like GDB or Programmer calls)
export PATH := $(RISCV_BIN):$(GDB_S_BIN):$(QP_BIN):$(AMP_BIN):$(PATH)

############################################
# Programs & Flags (Hard-coded Paths to avoid Error 127)
############################################
CC      := $(RISCV_BIN)/riscv32-unknown-elf-gcc
LD      := $(CC)
OD      := $(RISCV_BIN)/riscv32-unknown-elf-objdump
NM      := $(RISCV_BIN)/riscv32-unknown-elf-nm
RM      := rm -f

# Source and Include Paths
MAIN    := core/sw/src/main.c
SRCS    := $(wildcard core/sw/src/*.c)
HDRS    := $(wildcard core/sw/inc/*.h)
INCLUDES := -I. -Icore/sw/inc -IComputer_Systems/DE1-SoC/software

# RISC-V Architecture Flags
ARCH_FLAGS := -march=rv32im_zicsr -mabi=ilp32

# Compilation Flags
USERCCFLAGS := -g -O1 -ffunction-sections -fno-inline -gdwarf-2 $(INCLUDES)
# Note: Linux linker symbols do not use the double-dollar sign
USERLDFLAGS := -Wl,--defsym=__stack_pointer=0x4000000 \
               -Wl,--defsym,JTAG_UART_BASE=0xff201000 \
               -Wl,-e,main \
               -nostartfiles -lm

CFLAGS  := $(ARCH_FLAGS) $(USERCCFLAGS) -Wall
LDFLAGS := $(ARCH_FLAGS) $(USERLDFLAGS)

# Files
OBJS    := $(SRCS:.c=.o)
ELF     := $(MAIN:.c=.elf)

############################################
# Formatting
############################################
RED    := \033[31m
GREEN  := \033[32m
CYAN   := \033[36m
RESET  := \033[0m

############################################
# Compilation Targets

all: $(ELF)

$(ELF): $(OBJS)
	@echo -e "$(CYAN)Linking $@$(RESET)"
	$(LD) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c $(HDRS)
	@echo -e "$(GREEN)Compiling $<$(RESET)"
	$(CC) $(CFLAGS) -c $< -o $@

CLEAN:
	@echo -e "$(RED)Cleaning files...$(RESET)"
	$(RM) $(OBJS) $(ELF)

SYMBOLS: $(ELF)
	$(NM) -p $<

OBJDUMP: $(ELF)
	$(OD) -d -S $<

############################################
# System Targets

DE1-SoC:
	$(INSTALL)/qprogrammer/quartus/bin/quartus_pgm -c "DE-SoC" -m jtag -o "P;$(INSTALL)/fpgacademy/Computer_Systems/DE1-SoC/DE1-SoC_Computer/niosVg/DE1_SoC_Computer.sof@2"

TERMINAL:
	$(INSTALL)/fpgacademy/AMP/bin/nios2-terminal --instance 0

############################################
# GDB Targets

GDB_SERVER:
	$(GDB_S_BIN)/ash-riscv-gdb-server --device 02D120DD --gdb-port 2454 --instance 1 --probe-type USB-Blaster-2 --transport-type jtag --auto-detect true

GDB_CLIENT:
	$(RISCV_BIN)/riscv32-unknown-elf-gdb -silent "$(ELF)" \
	-ex "target remote:2454" \
	-ex "load" \
	-ex "set \$$sp=0x4000000" \
	-ex "set \$$pc=main" \
	-ex "info reg pc sp"

.PHONY: all CLEAN DE1-SoC GDB_SERVER GDB_CLIENT TERMINAL
