if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I. -I$(top_srcdir)/libecasound/include -I$(top_srcdir)/kvutils

if USE_NCURSES
termcap_library = -lncurses
else
termcap_library = -ltermcap
endif

if INCLUDE_DEBUG
bin_PROGRAMS = ecasound_debug
else
bin_PROGRAMS = ecasound
endif

ecasound_SOURCES = eca-text.cpp eca-text.h textdebug.cpp textdebug.h
ecasound_debug_SOURCES = $(ecasound_SOURCES)

ecasound_LDADD = \
		 $(top_srcdir)/libecasound/src/libecasound.la \
		 $(top_srcdir)/readline-4.0/libreadline.a $(top_srcdir)/readline-4.0/libhistory.a  \
                 $(termcap_library) \
		 -L$(top_srcdir)/kvutils \
		 -L$(top_srcdir)/kvutils/.libs -lkvutils
#		 -ltermcap
#		 -lreadline -lhistory
ecasound_debug_LDADD = $(top_srcdir)/readline-4.0/libreadline.a $(top_srcdir)/readline-4.0/libhistory.a \
		       $(top_srcdir)/libecasound/src/libecasound_debug.la \
		       $(termcap_library) \
		      -L$(top_srcdir)/kvutils \
		      -L$(top_srcdir)/kvutils/.libs -lkvutils
#		       -ltermcap
#			 /usr/lib/libreadline.a /usr/lib/libhistory.a
#ecasound_debug_LDFLAGS = -lkvutils
# ecasound_LDADD = -lm -lpthread $(top_srcdir)/libecasound/src/libecasound.la
# ../kvutils/.libs/libkvutils.a

