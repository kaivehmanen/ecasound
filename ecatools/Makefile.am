if ECA_AM_DEBUG_MODE
libecasound_path = $(top_builddir)/libecasound/libecasound@LIBECASOUND_VERSION@_debug.la \
	 	   $(top_builddir)/kvutils/libkvutils@LIBKVUTILS_VERSION@_debug.la
libecasoundc_path = $(top_builddir)/libecasoundc/libecasoundc@LIBECASOUNDC_VERSION@_debug.la
else
libecasound_path = $(top_builddir)/libecasound/libecasound@LIBECASOUND_VERSION@.la \
		   $(top_builddir)/kvutils/libkvutils@LIBKVUTILS_VERSION@.la
libecasoundc_path = $(top_builddir)/libecasoundc/libecasoundc@LIBECASOUNDC_VERSION@.la
endif

if ECA_AM_USE_NCURSES
ncurses_library = -lncurses
endif

if ECA_AM_USE_TERMCAP
termcap_library = -ltermcap
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/kvutils -I$(top_srcdir)/libecasound -I$(top_srcdir)/libecasoundc


if ECA_AM_DEBUG_MODE
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
ecalength_LDADD = $(libecasound_path) $(libecasoundc_path)

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

