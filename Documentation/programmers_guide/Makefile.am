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
	mkdir -p html_pguide
	latex2html -no_math -html_version 3.2,math -short_index -split 4 -link 5 -dir html_pguide programmers_guide.latex

ecidoc: eci_doc.latex
	latex eci_doc.latex
	mkdir -p html_ecidoc
	latex2html -no_math -html_version 3.2,math -short_index -split 4 -link 5 -dir html_ecidoc eci_doc.latex

clean-docs:
	rm -fv *.log *.toc *.dvi *.aux *.html
	rm -fv html_pguide/*
	rm -fv html_ecidoc/*

distclean: