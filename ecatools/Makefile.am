if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
libecasound_path = -L$(top_builddir)/kvutils -L$(top_builddir)/kvutils/.libs -lkvutils $(top_builddir)/libecasound/src/libecasound_debug.la
else
INSTALL_STRIP_FLAG = -s
libecasound_path = -L$(top_builddir)/kvutils -L$(top_builddir)/kvutils/.libs -lkvutils $(top_builddir)/libecasound/src/libecasound.la
endif


INCLUDES = -I$(srcdir) -I$(top_srcdir)/kvutils \
			-I$(top_srcdir)/libecasound/include \
			-I$(top_srcdir)/libecasound/src

if INCLUDE_DEBUG
bin_PROGRAMS = ecatools_normalize_debug \
	       ecatools_fixdc_debug \
	       ecatools_play_debug
else
bin_PROGRAMS = ecatools_normalize \
	       ecatools_fixdc \
	       ecatools_play
endif

ecatools_normalize_SOURCES = ecatools_normalize.cpp \
	 		     ecatools_normalize.h
ecatools_normalize_LDADD =  $(libecasound_path)

ecatools_normalize_LDADD = $(libecasound_path)

ecatools_fixdc_SOURCES = ecatools_fixdc.cpp \
			 ecatools_fixdc.h
ecatools_fixdc_LDADD = $(libecasound_path)

ecatools_play_SOURCES = ecatools_play.cpp \
			ecatools_play.h
ecatools_play_LDADD = $(libecasound_path)

ecatools_normalize_debug_SOURCES = $(ecatools_normalize_SOURCES)
ecatools_normalize_debug_LDADD = $(ecatools_normalize_LDADD)

ecatools_fixdc_debug_SOURCES = $(ecatools_fixdc_SOURCES)
ecatools_fixdc_debug_LDADD = $(ecatools_fixdc_LDADD)

ecatools_play_debug_SOURCES = $(ecatools_play_SOURCES)
ecatools_play_debug_LDADD = $(ecatools_play_LDADD)
