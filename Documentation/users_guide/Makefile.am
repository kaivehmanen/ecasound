# ----------------------------------------------------------------------
# File: ecasound/Documentation/users_guide/Makefile.am
# Description: Ecasound documentation - user's guide
# License: GPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

# ---------------------------------------------------------------------
# Files going into distribution:
EXTRA_DIST = users_guide.latex

all:

docs: ecasound_users_guide.pdf

ecasound_eci_doc.pdf: eci_doc.latex eci_doc.dvi

users_guide.dvi: users_guide.latex
	latex users_guide.latex
	dvips -Ppdf -o users_guide.ps users_guide.dvi
	ps2pdf users_guide.ps ecasound_users_guide.pdf
	mkdir -p html_uguide
	hevea -o html_uguide/users_guide.html users_guide.latex
	hevea -o html_uguide/users_guide.html users_guide.latex

ecasound_users_guide.pdf: users_guide.dvi

clean-docs:
	rm -fv users_guide.log users_guide.toc users_guide.dvi users_guide.aux users_guide.ps users_guide.dvi
	rm -fv html_uguide/*

