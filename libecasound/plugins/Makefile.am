AUTOMAKE_OPTIONS = foreign

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

#common_clags = -DECA_ENABLE_AUDIOIO_PLUGINS

if ECA_AM_COMPILE_ARTS
arts_libs = ${ECA_S_ARTSC_LIBS}
arts_cflags = ${ECA_S_ARTSC_CFLAGS}
arts_target = libaudioio_arts.la
else
arts_target =
endif

if ECA_AM_COMPILE_ALSA
alsa_libs = -lasound
alsa_target = libaudioio_alsa.la \
	      libaudioio_alsa_named.la
else
alsa_target = 
endif

if ECA_AM_COMPILE_AUDIOFILE
af_libs = -laudiofile
af_target = libaudioio_af.la
else
af_target = 
endif

if ECA_AM_COMPILE_JACK
jack_libs = -L$(ECA_S_JACK_LIBS) -ljack
jack_cflags = -I$(ECA_S_JACK_INCLUDES)
jack_target = libaudioio_jack.la
else
jack_target =
endif

INCLUDES =	-I$(srcdir) \
		-I$(top_srcdir) \
		-I$(top_srcdir)/libecasound \
		-I$(top_srcdir)/kvutils \
		$(arts_cflags) \
		$(jack_cflags)

#libdir = 	${exec_prefix}/lib/libecasound@LIBECASOUND_VERSION@-plugins

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

plugin_includes = 	\
			audioio_af.h \
			audioio_alsa.h \
			audioio_alsa_named.h \
			audioio_arts.h \
			audioio_jack.h \
			audioio_jack_manager.h

noinst_HEADERS =   	$(plugin_includes)

# ----------------------------------------------------------------------
# build targets and compiler options target defines
# ----------------------------------------------------------------------

if ECA_AM_DEBUG_MODE
noinst_LTLIBRARIES = libecasound_plugins_debug.la
else
noinst_LTLIBRARIES = libecasound_plugins.la
endif

#lib_LTLIBRARIES = 	$(af_target) \
#                       $(alsa_target) \
#			$(arts_target) \
#			$(jack_target)

# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

libecasound_plugins_la_SOURCES		= audioio_af.cpp \
                                          audioio_alsa.cpp audioio_alsa_named.cpp \
                                          audioio_arts.cpp \
                                          audioio_jack.cpp audioio_jack_manager.cpp
libecasound_plugins_la_LIBADD		= $(af_libs) $(alsa_libs) $(arts_libs) $(jack_libs)
libecasound_plugins_la_LDFLAGS 		= -static

libecasound_plugins_debug_la_SOURCES	= $(libecasound_plugins_la_SOURCES)
libecasound_plugins_debug_la_LIBADD	= $(libecasound_plugins_la_LIBADD)
libecasound_plugins_debug_la_LDFLAGS	= $(libecasound_plugins_la_LDFLAGS)

#libaudioio_af_la_SOURCES	     	= audioio_af.cpp
#libaudioio_alsa_la_SOURCES	     	= audioio_alsa.cpp audioio_alsa_hw.cpp
#libaudioio_alsa_named_la_SOURCES	= audioio_alsa.cpp audioio_alsa_named.cpp
#libaudioio_arts_la_SOURCES	        = audioio_arts.cpp
#libaudioio_jack_la_SOURCES	        = audioio_jack.cpp audioio_jack_manager.cpp

#libaudioio_af_la_LIBADD	     	= $(af_libs)
#libaudioio_alsa_la_LIBADD	     	= $(alsa_libs)
#libaudioio_alsa_named_la_LIBADD	= $(alsa_libs)
#libaudioio_arts_la_LIBADD		= 
#libaudioio_jack_la_LIBADD		= 

#libaudioio_af_la_LDFLAGS	     	= -avoid-version -export-dynamic
#libaudioio_alsa_la_LDFLAGS	     	= -avoid-version -export-dynamic
#libaudioio_alsa_named_la_LDFLAGS     	= -avoid-version -export-dynamic
#libaudioio_arts_la_LDFLAGS	        = -avoid-version -export-dynamic $(arts_libs)
#libaudioio_jack_la_LDFLAGS	        = -avoid-version -export-dynamic $(jack_libs)

# ----------------------------------------------------------------------
# separate rules for compiling non-libtool plugins
# ----------------------------------------------------------------------
