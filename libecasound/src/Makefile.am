if COMPILE_ALSA
alsa_libs = -lasound
endif

if COMPILE_AF
af_libs = -laudiofile
endif

if INCLUDE_DEBUG
eca_ldflags = -version-info 5:0:0 -lkvutils
else
eca_ldflags = -s -version-info 5:0:0 -lkvutils
endif

if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I$(top_srcdir)/include

INCLUDES = -I. -I$(top_srcdir) -I$(top_srcdir)/kvutils \
				-I$(top_srcdir)/libecasound/include

#SUBDIRS = kvutils
#DIST_SUBDIRS = kvutils
#CXXFLAGS += -D_REENTRANT -fomit-frame-pointer -funroll-all-loops \
#	   -finline-functions -ffast-math

# you can add here more. This one gets installed 

if INCLUDE_DEBUG
lib_LTLIBRARIES = libecasound_debug.la
else
lib_LTLIBRARIES = libecasound.la
endif

ecasound_audiofx_src =  audiofx.cpp \
			audiofx_amplitude.cpp \
			audiofx_compressor.cpp \
			audiofx_analysis.cpp \
			audiofx_filter.cpp \
			audiofx_rcfilter.cpp \
			audiofx_timebased.cpp \
			audiogate.cpp \
			audiofx_mixing.cpp \
			audiofx_ladspa.cpp

ecasound_audioio_oss_src = audioio-oss.cpp

ecasound_audioio_alsa_src = 	audioio-alsa.cpp \
				audioio-alsa2.cpp \
				audioio-alsalb.cpp

ecasound_audioio_src = 	audioio-cdr.cpp \
			audioio-ewf.cpp \
			audioio-mp3.cpp \
			audioio-wave.cpp \
			audioio.cpp \
			audioio-af.cpp \
			audioio-raw.cpp \
			audioio-mikmod.cpp \
			audioio-rtnull.cpp \
			audioio-types.cpp \
			audioio-loop.cpp \
			$(ecasound_audioio_oss_src) \
			$(ecasound_audioio_alsa_src)

ecasound_controller_src = \
	generic-controller.cpp \
	midi-cc.cpp \
	osc-gen.cpp \
	osc-sine.cpp \
	linear-envelope.cpp \
	two-stage-linear-envelope.cpp

ecasound_general_src = 	eca-chain.cpp \
			eca-main.cpp \
			eca-midi.cpp \
			eca-session.cpp \
			eca-resources.cpp \
			resource-file.cpp \
			eca-comhelp.cpp \
			eca-debug.cpp \
			layer.cpp \
			samplebuffer_iterators.cpp \
			eca-version.cpp  \
			eca-chainsetup.cpp \
			eca-controller.cpp \
			eca-controller-base.cpp \
			eca-controller-objects.cpp \
			eca-controller-dump.cpp \
			eca-iamode-parser.cpp \
			eca-control-position.cpp \
			eca-audio-position.cpp \
			eca-audio-objects.cpp \
			eca-audio-format.cpp \
			eca-audio-time.cpp \
			eca-fileio-stream.cpp \
			eca-fileio-mmap.cpp \
			eca-fileio-mmap-fthread.cpp \
			eca-alsa-dyn.cpp eca-mthreaded-processor.cpp \
			eca-static-object-maps.cpp \
			preset.cpp \
			file-preset.cpp \
			eca-object-map.cpp \
			eca-audio-object-map.cpp \
			eca-chainop-map.cpp \
			eca-controller-map.cpp \
			eca-preset-map.cpp \
			global-preset.cpp

#			eca-audio-object-map.cpp \
#			eca-chainop-map.cpp \
#			eca-controller-map.cpp \


ecasound_common_src = $(ecasound_audiofx_src) $(ecasound_audioio_src) \
		      $(ecasound_controller_src) $(ecasound_general_src)

#	              LeakTracer.cc

libecasound_la_SOURCES = $(ecasound_common_src)
libecasound_debug_la_SOURCES = $(ecasound_common_src)
#libecasound_la_LIBADD = -lm -lpthread -lkvutils $(af_libs) $(alsa_libs)
#libecasound_la_LIBADD = -lm -lpthread -ldl -lkvutils $(af_libs)
#libecasound_la_LIBADD = $(af_libs) $(top_srcdir)/kvutils/libkvutils.la
libecasound_la_LIBADD = $(af_libs) -L$(top_builddir)/kvutils -lkvutils
libecasound_debug_la_LIBADD = $(libecasound_la_LIBADD)
#libecasound_debug_la_LIBADD = $(libecasound_la_LIBADD) -lefence
libecasound_la_LDFLAGS = --export-dynamic $(eca_ldflags)
libecasound_debug_la_LDFLAGS = $(libecasound_la_LDFLAGS)

#libecasound_la_LDFLAGS = -release $(VERSION)








