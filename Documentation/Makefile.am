#
# Makefile for Documentation/ subdir 
#
# ---------------------------------------------------------------------
prefix = @prefix@
mandir = $(prefix)/man
PACKAGE = @PACKAGE@
VERSION = @VERSION@
SUBDIRS = users_guide programmers_guide

SCANDOC = /opt/p/scandoc/scandoc
#distdir = ../$(PACKAGE)-$(VERSION)
#top_distdir = $(distdir)
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = Makefile.am Makefile.in ecasound.1 qtecasound.1 \
	    ecasound_manpage.man qtecasound_manpage.man \
	    ecasound_manpage.html qtecasound_manpage.html \
	    ecasoundrc_manpage.man ecasoundrc_manpage.html \
	    ecatools_manpage.man ecatools_manpage.html \
	    ecasound-iam_manpage.man ecasound-iam_manpage.yo \
	    ecasound-iam.1 ecasound-iam_manpage.html \
	    ecatools.1 ecatools_manpage.yo \
            ecasoundrc.5 ecasound_faq.txt ecasound_faq.html \
	    ecasound_manpage.yo qtecasound_manpage.yo \
	    ecasoundrc_manpage.yo ecasound_faq.yo \
	    examples.html index.html

# ---------------------------------------------------------------------
docs: ecasound.1 qtecasound.1 ecasound_manpage.html qtecasound_manpage.html \
      ecasoundrc_manpage.html ecasoundrc.5 ecasound_faq.txt \
      ecatools.1 ecatools_manpage.man ecatools_manpage.html \
      ecasound_faq.html ecasound-iam_manpage.html \
      ecasound-iam.1

# ---------------------------------------------------------------------
# Man pages

ecasound.1: ecasound_manpage.man
	groff -man -Tascii ecasound_manpage.man >ecasound.1
ecatools.1: ecatools_manpage.man
	groff -man -Tascii ecatools_manpage.man >ecatools.1
qtecasound.1: qtecasound_manpage.man
	groff -man -Tascii qtecasound_manpage.man >qtecasound.1
ecasound-iam.1: ecasound-iam_manpage.man
	groff -man -Tascii ecasound-iam_manpage.man >ecasound-iam.1
ecasoundrc.5: ecasoundrc_manpage.man
	groff -man -Tascii ecasoundrc_manpage.man >ecasoundrc.5

ecasound_manpage.man: ecasound_manpage.yo
	yodl2man ecasound_manpage.yo
ecatools_manpage.man: ecatools_manpage.yo
	yodl2man ecatools_manpage.yo
qtecasound_manpage.man: qtecasound_manpage.yo
	yodl2man qtecasound_manpage.yo
ecasound-iam_manpage.man: ecasound-iam_manpage.yo
	yodl2man ecasound-iam_manpage.yo
ecasoundrc_manpage.man: ecasoundrc_manpage.yo
	yodl2man ecasoundrc_manpage.yo

ecasound_manpage.html: ecasound_manpage.yo
	yodl2html ecasound_manpage.yo
ecatools_manpage.html: ecatools_manpage.yo
	yodl2html ecatools_manpage.yo
qtecasound_manpage.html: qtecasound_manpage.yo
	yodl2html qtecasound_manpage.yo
ecasound-iam_manpage.html: ecasound-iam_manpage.yo
	yodl2html ecasound-iam_manpage.yo
ecasoundrc_manpage.html: ecasoundrc_manpage.yo
	yodl2html ecasoundrc_manpage.yo

# ---------------------------------------------------------------------
# FAQ, guides
ecasound_faq.txt: ecasound_faq.yo
	yodl2txt ecasound_faq.yo
ecasound_faq.html: ecasound_faq.yo
	yodl2html ecasound_faq.yo

# uguide: users_guide/users_guide.yo
# 	cd users_guide ; yodl2html users_guide.yo ; \
# 	ln -sf users_guide.html index.html ; cd ..
# pguide: programmers_guide/programmers_guide.yo
# 	cd programmers_guide ; yodl2html programmers_guide.yo ; \
# 	ln -sf programmers_guide.html index.html ;  cd ..

# ---------------------------------------------------------------------
# kdoc creation @top_srcdir@
kdoc:
	cd @top_srcdir@/kvutils ; \
	 kdoc -u \
	 -d ../Documentation/libkvutils/ \
	 -u "http://localhost/ecasound/Documentation/libkvutils/" \
	 -n libkvutils \
	 -L ../Documentation/ *.h
	cd @top_srcdir@/libecasound/include ; \
	 kdoc -f html -u "http://localhost/ecasound/Documentation/libecasound/" \
	 -d ../../Documentation/libecasound/ -n libecasound \
	 -L ../../Documentation/ *.h -llibkvutils
	cd @top_srcdir@/libqtecasound ; \
	 kdoc -f html -u "http://localhost/ecasound/Documentation/libqtecasound/" \
	 -d ../Documentation/libqtecasound/ -n libqtecasound \
	 -L ../Documentation/ *.h \
	 -llibecasound -llibkvutils -lqt
	cd @top_srcdir@/ecasound ; \
	 kdoc -f html -u \
	 "http://localhost/ecasound/Documentation/ecasound/" \
	 -d ../Documentation/ecasound/ \
	 -L ../Documentation/ *.h *.cpp -llibecasound
	cd @top_srcdir@/qtecasound ; \
	 kdoc -f html -u \
	 "http://localhost/ecasound/Documentation/qtecasound/" \
	 -d ../Documentation/qtecasound/ \
	 -L ../Documentation/ *.h -llibecasound -lqt -llibkvutils
	cd @top_srcdir@/Documentation/qtecasound ; \
	 qt2kdoc -u "http://www.troll.no/qt/" -o . /usr/doc/qt/
	cd @top_srcdir@/ecatools ; \
	 kdoc -f html -u \
	 "http://localhost/ecasound/Documentation/ecatools/" \
	 -d ../Documentation/ecatools/ \
	 -L ../Documentation/ *.h *.cpp -llibecasound

# ---------------------------------------------------------------------
# scandoc creation @top_srcdir@
scandoc:
	perl $(SCANDOC) -i scandoc_template_kvutils.pl -p test/ @top_srcdir@/kvutils/*.h
	perl $(SCANDOC) -i scandoc_template_libqtecasound.pl -p test3/ @top_srcdir@/libqtecasound/*.h
	perl $(SCANDOC) -i scandoc_template_libecasound.pl -p test2/ @top_srcdir@/libecasound/include/*.h

# ---------------------------------------------------------------------
# doxygen
doxygen:
	doxygen doxygen_libkvutils_config
	doxygen doxygen_libecasound_config
	doxygen doxygen_libqtecasound_config
	doxygen doxygen_qtecasound_config

# ---------------------------------------------------------------------
# Clean targets
clean:
clean-docs:
	rm -f ecasound.1 qtecasound.1 \
	      ecasound_manpage.html qtecasound_manpage.html \
	      ecasound_manpage.man qtecasound_manpage.man \
	      ecatools_manpage.man ecatools.1 \
	      ecatools_manpage.html \
	      ecasoundrc_manpage.man ecasoundrc_manpage.html \
	      ecasoundrc.5 design.html \
	      ecasound-iam.1 ecasound-iam_manpage.man ecasound-iam_manpage.html


# ---------------------------------------------------------------------
# Install targets
install-data-local:
	mkdir -p $(mandir)/man1
	mkdir -p $(mandir)/man5
	cp $(srcdir)/ecasound.1 $(mandir)/man1
	cp $(srcdir)/qtecasound.1 $(mandir)/man1
	cp $(srcdir)/ecatools.1 $(mandir)/man1
	cp $(srcdir)/ecasound-iam.1 $(mandir)/man1
	cp $(srcdir)/ecasoundrc.5 $(mandir)/man5
	cp $(srcdir)/ecasound_faq.txt ../$(topdir)/FAQ

# ---------------------------------------------------------------------
# Distdir
#distdir: $(DISTFILES)
#	distdir=`cd $(distdir) && pwd`; \
#	cp -a $(DISTFILES) $(distdir)
#	echo Distdir: $(distdir) ...

# --- 
# Autoconf magic
#Makefile: @top_srcdir@/Documentation/Makefile.in $(top_builddir)/config.status
#	cd $(top_builddir) \
#	&& CONFIG_FILES=Documentation/Makefile CONFIG_HEADERS= $(SHELL) ./config.status
