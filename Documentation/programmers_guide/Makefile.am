#
# Makefile for Documentation/programmers_guide subdir 
#
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = Makefile.am Makefile.in programmers_guide.yo programmers_guide.html

all:

pguide: programmers_guide.yo
	yodl2html programmers_guide.yo
	ln -sf programmers_guide.html index.html
