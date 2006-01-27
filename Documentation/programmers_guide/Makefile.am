#
# Makefile for Documentation/programmers_guide subdir 
#
# ---------------------------------------------------------------------
# Files going into distribution:
EXTRA_DIST =	ecasound_programmers_guide.txt \
		eci_doc.latex

all:

docs: ecasound_programmers_guide.html ecasound_eci_doc.pdf

ecasound_programmers_guide.html: ecasound_programmers_guide.txt
	rst2html ecasound_programmers_guide.txt ecasound_programmers_guide.html

ecasound_eci_doc.pdf: eci_doc.latex eci_doc.dvi
	latex eci_doc.latex
	dvips -Ppdf -o eci_doc.ps eci_doc.dvi
	ps2pdf eci_doc.ps ecasound_eci_doc.pdf
	mkdir -p html_ecidoc
	hevea -o html_ecidoc/eci_doc.html eci_doc.latex
	hevea -o html_ecidoc/eci_doc.html eci_doc.latex

eci_doc.dvi: eci_doc.latex

clean-docs:
	rm -fv ecasound_programmers_guide.html
	rm -fv eci_doc.log eci_doc.toc eci_doc.dvi eci_doc.aux eci_doc.ps eci_doc.dvi
	rm -fv html_ecidoc/*
