if ECA_AM_SYSTEM_READLINE
INCLUDES = -I$(ECA_S_READLINE_INCLUDES) -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/libecasound -I$(top_srcdir)/kvutils
else
INCLUDES = -I$(top_srcdir)/readline-4.0 -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/libecasound -I$(top_srcdir)/kvutils
endif

if ECA_AM_USE_NCURSES
termcap_library_ncurses = -lncurses
endif

if ECA_AM_USE_TERMCAP
termcap_library_termcap = -ltermcap
endif

termcap_library = $(termcap_library_ncurses) $(termcap_library_termcap)

if ECA_AM_DEBUG_MODE
bin_PROGRAMS = ecasound_debug
else
bin_PROGRAMS = ecasound
endif

ecasound_SOURCES = eca-text.cpp eca-text.h textdebug.cpp textdebug.h
ecasound_debug_SOURCES = $(ecasound_SOURCES)

if ECA_AM_SYSTEM_READLINE
ecasound_LDADD = $(termcap_library) \
		$(top_builddir)/libecasound/libecasound@LIBECASOUND_VERSION@.la \
		$(top_builddir)/kvutils/libkvutils@LIBKVUTILS_VERSION@.la
ecasound_debug_LDADD = $(termcap_library) \
	 	$(top_builddir)/libecasound/libecasound@LIBECASOUND_VERSION@_debug.la \
		$(top_builddir)/kvutils/libkvutils@LIBKVUTILS_VERSION@_debug.la
else
ecasound_LDADD = $(top_builddir)/readline-4.0/libreadline.a \
		$(top_builddir)/readline-4.0/libhistory.a $(termcap_library) \
		$(top_builddir)/libecasound/libecasound@LIBECASOUND_VERSION@.la \
		$(top_builddir)/kvutils/libkvutils@LIBKVUTILS_VERSION@.la
ecasound_debug_LDADD = $(top_builddir)/readline-4.0/libreadline.a \
		$(top_builddir)/readline-4.0/libhistory.a $(termcap_library) \
	 	$(top_builddir)/libecasound/libecasound@LIBECASOUND_VERSION@_debug.la \
		$(top_builddir)/kvutils/libkvutils@LIBKVUTILS_VERSION@_debug.la
endif
