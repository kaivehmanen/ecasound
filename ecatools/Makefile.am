if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
libecasound_path = -L$(top_builddir)/kvutils -L$(top_builddir)/kvutils/.libs -lkvutils $(top_builddir)/libecasound/libecasound_debug.la
else
INSTALL_STRIP_FLAG = -s
libecasound_path = -L$(top_builddir)/kvutils -L$(top_builddir)/kvutils/.libs -lkvutils $(top_builddir)/libecasound/libecasound.la
endif


INCLUDES = -I$(srcdir) -I$(top_srcdir)/kvutils \
			-I$(top_srcdir)/libecasound


if INCLUDE_DEBUG
bin_PROGRAMS = ecanormalize_debug \
	       ecafixdc_debug \
	       ecaplay_debug
else
bin_PROGRAMS = ecanormalize \
	       ecafixdc \
	       ecaplay
endif

ecanormalize_SOURCES = ecatools_normalize.cpp \
	 		     ecatools_normalize.h
ecanormalize_LDADD =  $(libecasound_path)

ecanormalize_LDADD = $(libecasound_path)

ecafixdc_SOURCES = ecatools_fixdc.cpp \
			 ecatools_fixdc.h
ecafixdc_LDADD = $(libecasound_path)

ecaplay_SOURCES = ecatools_play.cpp \
			ecatools_play.h
ecaplay_LDADD = $(libecasound_path)

ecanormalize_debug_SOURCES = $(ecanormalize_SOURCES)
ecanormalize_debug_LDADD = $(ecanormalize_LDADD)

ecafixdc_debug_SOURCES = $(ecafixdc_SOURCES)
ecafixdc_debug_LDADD = $(ecafixdc_LDADD)

ecaplay_debug_SOURCES = $(ecaplay_SOURCES)
ecaplay_debug_LDADD = $(ecaplay_LDADD)
