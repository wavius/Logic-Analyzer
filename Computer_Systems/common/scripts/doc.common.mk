# This Makefile contains the targets need to compile and release the computer system documentation

PDFS = $(TEXFILES:.tex=.pdf)

default: generate_pdfs

generate_pdfs: $(PDFS)

%.pdf: %.tex
	pdflatex $<
	pdflatex $<

release: $(PDFS)
	@echo Release $(PDFS)
	cp $(PDFS) $(CSROOT)/release/.

clean: 
	-rm -f *.aux *.log *.out *.toc *.pdf

.PHONY: generate_pdfs release clean
