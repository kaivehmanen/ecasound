#
# Makefile for Documentation/programmers_guide subdir 
#
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = 	Makefile.am \
		Makefile.in \
		programmers_guide.yo \
		programmers_guide.html \
		eci_doc.yo \
		eci_doc.html

all:

pguide: programmers_guide.yo
	yodl2html programmers_guide.yo
	ln -sf programmers_guide.html index.html

ecidoc: eci_doc.yo
	yodl2html eci_doc.yo

distclean: