if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
libecasound_path = $(top_builddir)/libecasound/libecasound_debug.la -L$(top_builddir)/kvutils/.libs -lkvutils
libecasoundc_path = $(top_builddir)/libecasoundc/libecasoundc_debug.la -L$(top_builddir)/kvutils/.libs -lkvutils
else
INSTALL_STRIP_FLAG = -s
libecasound_path = $(top_builddir)/libecasound/libecasound.la -L$(top_builddir)/kvutils/.libs -lkvutils
libecasoundc_path = $(top_builddir)/libecasoundc/libecasoundc.la -L$(top_builddir)/kvutils/.libs -lkvutils
endif

if USE_NCURSES
ncurses_library = -lncurses
endif

if USE_TERMCAP
termcap_library = -ltermcap
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/kvutils -I$(top_srcdir)/libecasound -I$(top_srcdir)/libecasoundc


if INCLUDE_DEBUG
bin_PROGRAMS = 	ecaconvert_debug \
		ecafixdc_debug \
	        ecalength_debug \
		ecanormalize_debug \
		ecaplay_debug \
		ecasignalview_debug
else
bin_PROGRAMS = 	ecaconvert \
		ecafixdc \
	        ecalength \
		ecanormalize \
		ecaplay \
		ecasignalview
endif

# --

ecaconvert_SOURCES = ecatools_convert.cpp \
			ecatools_convert.h
ecaconvert_LDADD = $(libecasound_path)

ecafixdc_SOURCES = ecatools_fixdc.cpp \
			 ecatools_fixdc.h
ecafixdc_LDADD = $(libecasound_path)

ecalength_SOURCES = ecalength.c
ecalength_LDADD = $(libecasoundc_path)

ecanormalize_SOURCES = ecatools_normalize.cpp \
	 		     ecatools_normalize.h
ecanormalize_LDADD = $(libecasound_path)

ecaplay_SOURCES = ecatools_play.cpp \
			ecatools_play.h
ecaplay_LDADD = $(libecasound_path)

ecasignalview_SOURCES = ecatools_signalview.cpp \
	 		     ecatools_signalview.h
ecasignalview_LDADD =  $(libecasound_path) $(termcap_library) $(ncurses_library)

# --

ecaconvert_debug_SOURCES = $(ecaconvert_SOURCES)
ecaconvert_debug_LDADD = $(ecaconvert_LDADD)

ecafixdc_debug_SOURCES = $(ecafixdc_SOURCES)
ecafixdc_debug_LDADD = $(ecafixdc_LDADD)

ecalength_debug_SOURCES = $(ecalength_SOURCES)
ecalength_debug_LDADD = $(ecalength_LDADD)

ecanormalize_debug_SOURCES = $(ecanormalize_SOURCES)
ecanormalize_debug_LDADD = $(ecanormalize_LDADD)

ecaplay_debug_SOURCES = $(ecaplay_SOURCES)
ecaplay_debug_LDADD = $(ecaplay_LDADD)

ecasignalview_debug_SOURCES = $(ecasignalview_SOURCES)
ecasignalview_debug_LDADD = $(ecasignalview_LDADD)

