# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4a from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

#
# Makefile for Documentation/ subdir 
#
# ---------------------------------------------------------------------


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/ecawave
pkglibdir = $(libdir)/ecawave
pkgincludedir = $(includedir)/ecawave

top_builddir = ..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
INSTALL_STRIP_FLAG =
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
CC = gcc
CPP = gcc -E
CXX = c++
MAKEINFO = makeinfo
MOC = /usr/lib/qt-2.0.1/bin/moc
QT_INCLUDES = -I/usr/lib/qt-2.0.1/include
QT_LDFLAGS = -L/usr/lib/qt-2.0.1/lib
qt_includes = /usr/lib/qt-2.0.1/include
qt_libraries = /usr/lib/qt-2.0.1/lib

prefix = /usr/local
mandir = $(prefix)/man
PACKAGE = ecawave
VERSION = 0.2.0d2   
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = Makefile.am 		Makefile.in 		ecawave.1 	    	ecawave_manpage.man 		ecawave_manpage.html 		ecawave_manpage.yo 	    	users_guide.yo 	    	users_guide.html

mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
DIST_COMMON =  Makefile.am Makefile.in


TAR = gtar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps Documentation/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status

tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = Documentation

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am:
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am:
uninstall: uninstall-am
all-am: Makefile
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) INSTALL_STRIP_FLAG=-s install
installdirs:


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: tags distdir info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


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
# kdoc creation ..
kdoc:
	cd ../src ; \
	 kdoc -f html -u \
	 "http://www.wakkanet.fi/~kaiv/ecawave/Documentation/source-tree/" \
	 -d ../Documentation/source-tree/ \
	 -L ../Documentation/ *.h -lqt -llibecasound -llibkvutils
	cd ../Documentation/source-tree ; \
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

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
