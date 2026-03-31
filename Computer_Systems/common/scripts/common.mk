# Check if the Makefile is running on WSL or directly in Linux
ifeq ($(origin WSL_DISTRO_NAME),environment)
    EXE := .exe
else
    EXE := 
endif

QSYSOBJS = $(TCLSRCS:.tcl=.qsys)
SOPCOBJS = $(QSYSSRC:.qsys=.sopcinfo)
CURPATH  = $(shell pwd)

default: generate_qsys_files

all: generate_qsys_files run_platform_designer reduce_sopcinfo fix_for_niosVg run_quartus generate_rbf release grep_for_errors

continue: run_platform_designer reduce_sopcinfo fix_for_niosVg run_quartus generate_rbf release grep_for_errors

generate_qsys_files: $(QSYSOBJS)

%.tcl: # dummy make target
	
%.qsys: %.tcl
	qsys-script$(EXE) --script=$(S2CPATH)/$< > $(CURPATH)/o_$<.txt 2>&1

run_platform_designer: $(QSYSSRC)
	mkdir -p $(DSTPATH)
	cp *.qsys $(DSTPATH)
	@echo -e "\033[33mClearing Platform Designer Cache...\033[0m"
	rm -rf $(CURPATH)/.qsys_edit $(DSTPATH)/.qsys_edit
	@echo -e "\033[36mGenerating Qsys System...\033[0m"
	cd $(DSTPATH) && qsys-generate$(EXE) ./$< --synthesis=VERILOG --search-path="$$,$(CURPATH)/**/*,$(CURPATH)/$(SRCPATH)**/*" > $(CURPATH)/o_$<.txt 2>&1

reduce_sopcinfo:
	cd $(DSTPATH) && python3 $(D2CPATH)/strip_info.py Computer_System.sopcinfo

fix_for_niosVg: # need to repair NiosVg due to platform designer bug
ifeq ($(findstring NiosVg,$(QP_NAME)), NiosVg)
	sed -i 's/000000-/1111111/' $(DSTPATH)/Computer_System/synthesis/submodules/Computer_System_NiosVg.v
endif

run_quartus: $(QP_NAME)
	cp -r $(SRCPATH) $(DSTPATH)
	cd $(DSTPATH) && quartus_sh$(EXE) --64bit --flow compile $< > $(CURPATH)/o_$<.txt 2>&1

generate_rbf: $(QP_NAME)
ifeq ($(GEN_RBF), 1)
	cd $(DSTPATH) && quartus_cpf$(EXE) -m FPP -o bitstream_compression=on -c $<.sof $<.rbf > $(CURPATH)/o_$<.rbf.txt 2>&1
endif

release:
	mkdir -p $(RELPATH)
	cp $(DSTPATH)/*.sopcinfo $(RELPATH)
	cp $(DSTPATH)/*.amp $(RELPATH)
	cp $(DSTPATH)/*.sof $(RELPATH)
ifeq ($(GEN_RBF), 1)
	cp $(DSTPATH)/*.rbf $(RELPATH)
endif


grep_for_errors:
	@grep Error *.txt || true 

clean:
	rm -f *.qsys
	rm -f o_*.txt

.PHONY: default all continue generate_qsys_files run_platform_designer run_quartus $(QP_NAME) release grep_for_errors clean

