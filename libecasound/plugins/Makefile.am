AUTOMAKE_OPTIONS = foreign

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if COMPILE_ARTS
arts_libs = `artsc-config --libs`
arts_cflags = `artsc-config --cflags`
else
arts_libs =
arts_cflags =
endif

if COMPILE_ALSA
alsa_libs = -lasound
endif

if COMPILE_AF
af_libs = -laudiofile
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/libecasound -I$(top_srcdir)/kvutils $(arts_cflags)
libdir = ${exec_prefix}/lib/ecasound-plugins

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

plugin_includes = 	\
			audioio_af.h \
			audioio_alsa.h \
			audioio_alsalb.h \
			audioio_alsa2.h \
			audioio_alsa2_plugin.h \
			audioio_alsa3.h \
			audioio_alsa3_pcm.h \
			audioio_arts.h

noinst_HEADERS = $(plugin_includes)

# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

lib_LTLIBRARIES = 	libaudioio_af.la \
			libaudioio_alsa.la \
			libaudioio_alsalb.la \
			libaudioio_alsa2.la \
			libaudioio_alsa2_plugin.la \
			libaudioio_alsa3.la \
			libaudioio_alsa3_pcm.la \
			libaudioio_arts.la

libaudioio_af_la_SOURCES	     	= audioio_af.cpp
libaudioio_alsa_la_SOURCES	     	= audioio_alsa.cpp
libaudioio_alsalb_la_SOURCES	     	= audioio_alsalb.cpp
libaudioio_alsa2_la_SOURCES     	= audioio_alsa2.cpp
libaudioio_alsa2_plugin_la_SOURCES 	= audioio_alsa2_plugin.cpp
libaudioio_alsa3_la_SOURCES		= audioio_alsa3.cpp audioio_alsa3_hw.cpp
libaudioio_alsa3_pcm_la_SOURCES		= audioio_alsa3.cpp audioio_alsa3_pcm.cpp
libaudioio_arts_la_SOURCES	        = audioio_arts.cpp

libaudioio_af_la_LIBADD	     		= $(af_libs)
libaudioio_alsa_la_LIBADD	     	= $(alsa_libs)
libaudioio_alsalb_la_LIBADD	     	= $(alsa_libs)
libaudioio_alsa2_la_LIBADD     		= $(alsa_libs)
libaudioio_alsa2_plugin_la_LIBADD 	= $(alsa_libs)
libaudioio_alsa3_la_LIBADD		= $(alsa_libs)
libaudioio_alsa3_pcm_la_LIBADD		= $(alsa_libs)
libaudioio_arts_la_LIBADD		= 

libaudioio_af_la_LDFLAGS	     	= -avoid-version -export-dynamic
libaudioio_alsa_la_LDFLAGS	     	= -avoid-version -export-dynamic
libaudioio_alsalb_la_LDFLAGS	     	= -avoid-version -export-dynamic
libaudioio_alsa2_la_LDFLAGS     	= -avoid-version -export-dynamic
libaudioio_alsa2_plugin_la_LDFLAGS 	= -avoid-version -export-dynamic
libaudioio_alsa3_la_LDFLAGS		= -avoid-version -export-dynamic
libaudioio_alsa3_pcm_la_LDFLAGS		= -avoid-version -export-dynamic
libaudioio_arts_la_LDFLAGS	        = -avoid-version -export-dynamic $(arts_libs)

# ----------------------------------------------------------------------
# separate rules for compiling non-libtool plugins
# ----------------------------------------------------------------------
