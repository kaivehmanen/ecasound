#
# Makefile for Documentation/users_guide subdir 
#
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = Makefile.am Makefile.in users_guide.latex

all:

docs: users_guide

users_guide: users_guide.latex
	latex users_guide.latex
	mkdir -p html_uguide
	latex2html -no_math -html_version 3.2,math -short_index -split 4 -link 5 -dir html_uguide users_guide.latex

clean-docs:
	rm -fv *.log *.toc *.dvi *.aux *.html
	rm -fv html_uguide/*

distclean: