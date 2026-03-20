# This Makefile calls the Makefiles for all computer systems
# It was run in Ubuntu 20.04 WSL with the package texlive-full on Windows 10
#
# Targets:
#
# 1) 
# Name:              default
# Command Line Call: make -f doc.mk
# Description:       Build all the Computer System PDFs from their Latex source
#
# 2)
# Name:              release
# Command Line Call: make -f doc.mk release
# Description:       Copy all the PDFs to a release directory
#
# 3)
# Name:              clean
# Command Line Call: make -f doc.mk clean
# Description:       Clean up the temporary files from all the Computer System directories
#
#
# To build and release individual PDFs, navigate to the specific computer system's
# directory and run:
# make -f doc.mk
# make -f doc.mk release
# make -f doc.mk clean
#


MAKEFILES = $(dir $(wildcard */*/*/doc.mk))


default: $(MAKEFILES)

$(MAKEFILES):
	$(MAKE) -C $@ -f doc.mk

dump:
	echo $(MAKEFILES)


release: mk_release_dir $(addprefix release.,$(MAKEFILES))

mk_release_dir:
	rm -rf ./release
	mkdir ./release

$(addprefix release.,$(MAKEFILES)): release.%:
	-$(MAKE) -C $* -f doc.mk -s release


clean: $(addprefix clean.,$(MAKEFILES))

$(addprefix clean.,$(MAKEFILES)): clean.%:
	-$(MAKE) -C $* -f doc.mk -s clean

.PHONY: default $(MAKEFILES) release mk_release_dir $(addprefix release.,$(MAKEFILES)) clean $(addprefix clean.,$(MAKEFILES))
