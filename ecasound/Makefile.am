if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir)/libecasound -I$(top_srcdir)/kvutils

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

ecasound_LDADD = $(top_builddir)/readline-4.0/libreadline.a $(top_builddir)/readline-4.0/libhistory.a \
                 $(termcap_library) \
		 $(top_builddir)/libecasound/libecasound.la \
		 -L$(top_builddir)/kvutils/.libs -lkvutils
ecasound_debug_LDADD = $(top_builddir)/readline-4.0/libreadline.a $(top_builddir)/readline-4.0/libhistory.a \
			$(termcap_library) \
		 	$(top_builddir)/libecasound/libecasound_debug.la \
			-L$(top_builddir)/kvutils/.libs -lkvutils

