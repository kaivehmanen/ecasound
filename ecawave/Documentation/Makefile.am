#
# Makefile for Documentation/ subdir 
#
# ---------------------------------------------------------------------
prefix = @prefix@
mandir = $(prefix)/man
PACKAGE = @PACKAGE@
VERSION = @VERSION@   
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = 	Makefile.am \
		Makefile.in \
		ecawave.1 \
	    	ecawave_manpage.man \
		ecawave_manpage.html \
		ecawave_manpage.yo \
	    	users_guide.yo \
	    	users_guide.html

# ---------------------------------------------------------------------
all: docs

distclean:

docs: 	ecawave.1 \
	ecawave_manpage.man \
	ecawave_manpage.html \
	users_guide.yo \
	users_guide.html

# ---------------------------------------------------------------------
# Man pages
ecawave.1: ecawave_manpage.man
	groff -man -Tascii ecawave_manpage.man >ecawave.1

ecawave_manpage.man: $(srcdir)/ecawave_manpage.yo
	yodl2man $(srcdir)/ecawave_manpage.yo

ecawave_manpage.html: $(srcdir)/ecawave_manpage.yo
	yodl2html $(srcdir)/ecawave_manpage.yo

users_guide.html: users_guide.yo
	yodl2html users_guide.yo

# ---------------------------------------------------------------------
# kdoc creation @top_srcdir@
kdoc:
	cd @top_srcdir@/src ; \
	 kdoc -f html -u \
	 "http://www.wakkanet.fi/~kaiv/ecawave/Documentation/source-tree/" \
	 -d ../Documentation/source-tree/ \
	 -L ../Documentation/ *.h -lqt -llibecasound -llibkvutils
	cd @top_srcdir@/Documentation/source-tree ; \
	 qt2kdoc -u "http://www.troll.no/qt/" -o . /usr/local/qt/html/

# ---------------------------------------------------------------------
# Clean targets
clean:
clean-docs:
	rm -f ecawave.1 \
	      ecawave_manpage.html \
	      ecawave_manpage.man \
	      users_guide.html

# ---------------------------------------------------------------------
# Install targets
install:
	mkdir -p $(mandir)/man1
	cp ecawave.1 $(mandir)/man1

# ---------------------------------------------------------------------
# Distdir
# distdir: $(DISTFILES)
#	distdir=`cd $(distdir) && pwd`; \
#	cp -a $(DISTFILES) $(distdir)
#	echo Distdir: $(distdir) ...

# --- 
# Autoconf magic
# Makefile: Makefile.in  ../config.status
#	cd .. \
#	&& CONFIG_FILES=Documentation/Makefile CONFIG_HEADERS= $(SHELL) ./config.status
