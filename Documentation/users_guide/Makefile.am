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
	dvips -Ppdf -o users_guide.ps users_guide.dvi
	ps2pdf users_guide.ps ecasound_users_guide.pdf
	mkdir -p html_uguide
	hevea -o html_uguide/users_guide.html users_guide.latex
	hevea -o html_uguide/users_guide.html users_guide.latex

clean-docs:
	rm -fv users_guide.log users_guide.toc users_guide.dvi users_guide.aux users_guide.ps users_guide.dvi
	rm -fv html_uguide/*

distclean: