EXTRA_DIST = ChangeLog
AUTOMAKE_OPTIONS = foreign

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if COMPILE_ALSA
alsa_libs = -lasound
endif

if COMPILE_AF
af_libs = -laudiofile
endif

if INCLUDE_DEBUG
eca_ldflags = -version-info 6:0:0 -lkvutils
else
eca_ldflags = -s -version-info 6:0:0 -lkvutils
endif

if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/kvutils

if INCLUDE_DEBUG
lib_LTLIBRARIES = libecasound_debug.la
else
lib_LTLIBRARIES = libecasound.la
endif

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

ecasound_audiofx_src =  audiofx.h \
			audiofx_impl.h \
			audiofx_amplitude.h \
			audiofx_compressor.h \
			audiofx_analysis.h \
			audiofx_filter.h \
			audiofx_rcfilter.h \
			audiofx_reverb.h \
			audiofx_timebased.h \
			audiogate.h \
			audiofx_mixing.h \
			audiofx_ladspa.h \
			ladspa.h \
			audiofx_vst.h

ecasound_audioio_src = 	audioio-cdr.h \
			audioio-cdr_impl.h \
			audioio-ewf.h \
			audioio-mp3.h \
			audioio-mp3_impl.h \
			audioio-wave.h \
			audioio.h \
			audioio-af.h \
			audioio-raw.h \
			audioio-oss.h \
			audioio-oss_impl.h \
			audioio-alsalb.h \
			audioio-alsa.h \
			audioio-null.h \
			audioio-rtnull.h \
			audioio-mikmod.h \
			audioio-types.h \
			audioio-alsa2.h \
			audioio-alsa2-plugin.h \
			audioio-alsa3.h \
			audioio-loop.h

ecasound_controller_src = \
	generic-controller.h \
	ctrl-source.h \
	midi-cc.h \
	oscillator.h \
	osc-gen.h \
	osc-sine.h \
	finite-envelope.h \
	linear-envelope.h \
	two-stage-linear-envelope.h

ecasound_general_src = 	eca-chain.h \
			eca-chainop.h \
			eca-error.h \
			eca-debug.h \
			eca-comhelp.h \
			eca-midi.h \
			eca-main.h \
			eca-session.h \
			eca-resources.h \
			resource-file.h \
			generic-controller.h \
			layer.h \
			eca-static-object-maps.h \
			eca-object-map.h \
			eca-preset-map.h \
			eca-ladspa-plugin-map.h \
			eca-vst-plugin-map.h \
			eca-chainop-map.h \
			eca-controller-map.h \
			eca-audio-object-map.h \
			samplebuffer.h \
			samplebuffer_impl.h \
			samplebuffer_functions.h \
			samplebuffer_iterators.h \
			sample-specs.h \
			eca-version.h \
			eca-chainsetup.h \
			eca-controller.h \
			eca-controller-base.h \
			eca-controller-objects.h \
			eca-controller-dump.h \
			eca-iamode-parser.h \
			osc-gen.h \
			midi-cc.h \
			eca-control-position.h \
			eca-audio-position.h \
			eca-fileio.h  \
			eca-fileio-stream.h \
			eca-fileio-mmap.h \
			eca-fileio-mmap-fthread.h \
			eca-mthreaded-processor.h \
			dynamic-parameters.h \
			dynamic-object.h \
			eca-object.h \
			eca-operator.h \
			eca-audio-objects.h \
			eca-audio-format.h \
			eca-audio-time.h \
			preset.h \
			file-preset.h \
			global-preset.h

if FEELING_EXPERIMENTAL
vst_header_files = AEffEditor.hpp AEffect.h aeffectx.h AudioEffect.hpp
endif

noinst_HEADERS = $(ecasound_audiofx_src) $(ecasound_audioio_src) \
		 $(ecasound_controller_src) $(ecasound_general_src) $(vst_header_files)


# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

if FEELING_EXPERIMENTAL
vst_src_files = AudioEffect.cpp audioeffectx.cpp
endif

ecasound_audiofx_src =  audiofx.cpp \
			audiofx_amplitude.cpp \
			audiofx_compressor.cpp \
			audiofx_analysis.cpp \
			audiofx_filter.cpp \
			audiofx_rcfilter.cpp \
			audiofx_reverb.cpp \
			audiofx_timebased.cpp \
			audiogate.cpp \
			audiofx_mixing.cpp \
			audiofx_ladspa.cpp \
			audiofx_vst.cpp

ecasound_audioio_oss_src = audioio-oss.cpp

ecasound_audioio_alsa_src = 	audioio-alsa.cpp \
				audioio-alsa2.cpp \
				audioio-alsa2-plugin.cpp \
				audioio-alsa3.cpp \
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
			eca-mthreaded-processor.cpp \
			eca-static-object-maps.cpp \
			preset.cpp \
			file-preset.cpp \
			eca-object-map.cpp \
			eca-audio-object-map.cpp \
			eca-chainop-map.cpp \
			eca-controller-map.cpp \
			eca-preset-map.cpp \
			eca-vst-plugin-map.cpp \
			global-preset.cpp

ecasound_common_src = 	$(ecasound_audiofx_src) \
			$(ecasound_audioio_src) \
		      	$(ecasound_controller_src) \
			$(ecasound_general_src) $(vst_src_files)

libecasound_la_SOURCES = $(ecasound_common_src)
libecasound_debug_la_SOURCES = $(ecasound_common_src)
libecasound_la_LIBADD = $(af_libs)  $(alsa_libs) -L$(top_builddir)/kvutils -lkvutils
libecasound_debug_la_LIBADD = $(libecasound_la_LIBADD)
libecasound_la_LDFLAGS = -export-dynamic $(eca_ldflags)
libecasound_debug_la_LDFLAGS = $(libecasound_la_LDFLAGS)

install-data-local:
	$(INSTALL) -d $(pkgincludedir)
	cd $(srcdir) ; \
	cp 	$(ecasound_audiofx_src) \
		$(ecasound_audioio_src) \
		$(ecasound_controller_src) \
		$(ecasound_general_src) $(vst_header_files) \
		$(pkgincludedir)
