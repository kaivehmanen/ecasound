AUTOMAKE_OPTIONS = foreign

# ---
# Makefile.am for building libkvutils.a
# ---
#CXXFLAGS += -D_REENTRANT -fomit-frame-pointer -funroll-all-loops -s \
#	     -finline-functions -ffast-math -I.
CXXFLAGS+=-D_REENTRANT -s -O2 -ffast-math -I.
# ---
# You can add here more. This one gets installed 
# ---
#noinst_LIBRARIES = libkvutils.a
lib_LTLIBRARIES = libkvutils.la

kvutil_sources = kvutils.cpp message_item.cpp com_line.cpp command_queue.cpp \
		 value_queue.cpp kvu_numtostr.cpp

kvutil_headers = kvutils.h message_item.h com_line.h \
		 command_queue.h value_queue.h \
		 kvu_numtostr.h definition_by_contract.h

#libkvutils_a_SOURCES = $(kvutil_sources) $(kvutil_headers)
libkvutils_la_SOURCES = $(kvutil_sources) $(kvutil_headers)
#libkvutils_la_LDFLAGS = -static
#libkvutils_la_LIBADD = -lpthread
#libkvutils_la_LIBADD = -lpth

DIST_COMMON =  README COPYING INSTALL Makefile.in Makefile.am \
aclocal.m4 config.guess config.sub configure configure.in \
install-sh mkinstalldirs ltconfig ltmain.sh

#nodist_COMMON =  AUTHORS NEWS ChangeLog 
#EXTRA_DIST = Makefile

noinst_HEADERS = $(kvutil_headers)

install-data-local:
	$(INSTALL) -d $(pkgincludedir)
	cp  $(kvutil_headers) $(pkgincludedir)





