#
# Makefile for Documentation/users_guide subdir 
#
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = Makefile.am Makefile.in users_guide.yo users_guide.html

all:
uguide: users_guide.yo
	yodl2html users_guide.yo
	ln -sf users_guide.html index.html
