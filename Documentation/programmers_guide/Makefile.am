#
# Makefile for Documentation/programmers_guide subdir 
#
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = 	Makefile.am \
		Makefile.in \
		programmers_guide.latex \
		eci_doc.latex

all:

docs: programmers_guide ecidoc 

programmers_guide: programmers_guide.latex
	latex programmers_guide.latex
	dvips -Ppdf -o programmers_guide.ps programmers_guide.dvi
	ps2pdf programmers_guide.ps ecasound_programmers_guide.pdf
	mkdir -p html_uguide
	latex2html -no_math -html_version 3.2,math -short_index -split 2 -link 5 -dir html_uguide programmers_guide.latex

ecidoc: eci_doc.latex
	latex eci_doc.latex
	dvips -Ppdf -o eci_doc.ps eci_doc.dvi
	ps2pdf eci_doc.ps ecasound_eci_doc.pdf
	mkdir -p html_ecidoc
	latex2html -no_math -html_version 3.2,math -short_index -split 2 -link 5 -dir html_ecidoc eci_doc.latex

clean-docs:
	rm -fv programmers_guide.log programmers_guide.toc programmers_guide.dvi programmers_guide.aux programmers_guide.ps programmers_guide.dvi
	rm -fv eci_doc.log eci_doc.toc eci_doc.dvi eci_doc.aux eci_doc.ps eci_doc.dvi
	rm -fv html_pguide/*
	rm -fv html_ecidoc/*

distclean: