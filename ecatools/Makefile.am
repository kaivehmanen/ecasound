if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
libecasound_path = $(top_builddir)/libecasound/libecasound_debug.la -L$(top_builddir)/kvutils/.libs -lkvutils
else
INSTALL_STRIP_FLAG = -s
libecasound_path = $(top_builddir)/libecasound/libecasound.la -L$(top_builddir)/kvutils/.libs -lkvutils
endif

if USE_NCURSES
termcap_library = -lncurses
endif

if USE_TERMCAP
termcap_library = -ltermcap
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/kvutils -I$(top_srcdir)/libecasound


if INCLUDE_DEBUG
bin_PROGRAMS = ecanormalize_debug \
	       ecafixdc_debug \
	       ecaplay_debug \
	       ecaconvert_debug \
	       ecasignalview_debug
else
bin_PROGRAMS = ecanormalize \
	       ecafixdc \
	       ecaplay \
	       ecaconvert \
	       ecasignalview
endif

ecanormalize_SOURCES = ecatools_normalize.cpp \
	 		     ecatools_normalize.h
ecanormalize_LDADD = $(libecasound_path)

ecafixdc_SOURCES = ecatools_fixdc.cpp \
			 ecatools_fixdc.h
ecafixdc_LDADD = $(libecasound_path)

ecaplay_SOURCES = ecatools_play.cpp \
			ecatools_play.h
ecaplay_LDADD = $(libecasound_path)

ecaconvert_SOURCES = ecatools_convert.cpp \
			ecatools_convert.h
ecaconvert_LDADD = $(libecasound_path)

ecasignalview_SOURCES = ecatools_signalview.cpp \
	 		     ecatools_signalview.h
ecasignalview_LDADD =  $(libecasound_path) $(termcap_library)

ecanormalize_debug_SOURCES = $(ecanormalize_SOURCES)
ecanormalize_debug_LDADD = $(ecanormalize_LDADD)

ecafixdc_debug_SOURCES = $(ecafixdc_SOURCES)
ecafixdc_debug_LDADD = $(ecafixdc_LDADD)

ecaplay_debug_SOURCES = $(ecaplay_SOURCES)
ecaplay_debug_LDADD = $(ecaplay_LDADD)

ecaconvert_debug_SOURCES = $(ecaconvert_SOURCES)
ecaconvert_debug_LDADD = $(ecaconvert_LDADD)

ecasignalview_debug_SOURCES = $(ecasignalview_SOURCES)
ecasignalview_debug_LDADD = $(ecasignalview_LDADD)

