EXTRA_DIST = ChangeLog
AUTOMAKE_OPTIONS = foreign
SUBDIRS = plugins

# ----------------------------------------------------------------------
# build targets and compiler options target defines
# ----------------------------------------------------------------------

if ECA_AM_DEBUG_MODE
lib_LTLIBRARIES = libecasound8_debug.la
else
lib_LTLIBRARIES = libecasound8.la
endif

TESTS = libecasound_tester

check_PROGRAMS = libecasound_tester

# ----------------------------------------------------------------------
# compiler and linker options
# ----------------------------------------------------------------------

if ECA_AM_ALL_STATIC
# FIXME: add linking to ecasound plugins
eca_libadd =
else
eca_libadd =
endif

eca_ldflags = -version-info @LIBECASOUND_VERSION@:0:0

if ECA_AM_DEBUG_MODE
libecasound_tester_libs =	$(top_builddir)/libecasound/.libs/libecasound@LIBECASOUND_VERSION@_debug.a \
				$(top_builddir)/kvutils/.libs/libkvutils@LIBKVUTILS_VERSION@_debug.a
else
libecasound_tester_libs =  	$(top_builddir)/libecasound/.libs/libecasound@LIBECASOUND_VERSION@.a \
				$(top_builddir)/kvutils/.libs/libkvutils@LIBKVUTILS_VERSION@.a
endif

# note! Automake >= 1.5 will install stripped libraries
#       with "make install-strip". Older versions won't
#       strip libraries even if INSTALL_STRIP_FLAG is set.
#       also libtool 1.3.2 and older have problems 
#       with stripping libraries.

INCLUDES = -I$(srcdir) -I$(top_srcdir)/kvutils

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

ecasound_audiofx_include =  \
			audiofx.h \
			audiofx_misc.h \
			audiofx_amplitude.h \
			audiofx_compressor.h \
			audiofx_analysis.h \
			audiofx_envelope_modulation.h \
			audiofx_filter.h \
			audiofx_rcfilter.h \
			audiofx_reverb.h \
			audiofx_timebased.h \
			audiogate.h \
			audiofx_mixing.h \
			audiofx_ladspa.h \
			audio-stamp.h

ecasound_audioio_include = 	\
			audioio.h \
			audioio-buffered.h \
			audioio-device.h \
			audioio-plugin.h \
			audioio-manager.h \
			audioio-cdr.h \
			audioio-cdr_impl.h \
			audioio-ewf.h \
			audioio-mp3.h \
			audioio-mp3_impl.h \
			audioio-ogg.h \
			audioio-wave.h \
			audioio-raw.h \
			audioio-oss.h \
			audioio-oss_impl.h \
			audioio-null.h \
			audioio-rtnull.h \
			audioio-mikmod.h \
			audioio-loop.h \
			audioio-forked-stream.h \
			audioio-timidity.h \
			audioio-proxy-server.h \
			audioio-proxy-server_impl.h \
			audioio-proxy-buffer.h \
			audioio-buffered-proxy.h \
			audioio-typeselect.h \
			audioio-reverse.h

ecasound_midi_include =	\
			midi-server.h \
			midi-client.h \
			midi-parser.h \
			midiio.h \
			midiio-raw.h

ecasound_controller_include = \
			generic-controller.h \
			ctrl-source.h \
			midi-cc.h \
			oscillator.h \
			osc-gen.h \
			osc-gen-file.h \
			osc-sine.h \
			linear-envelope.h \
			two-stage-linear-envelope.h \
			stamp-ctrl.h \
	 		generic-linear-envelope.h

ecasound_general_include = 	\
			eca-chain.h \
			eca-chainop.h \
			eca-error.h \
			eca-logger.h \
			eca-logger-default.h \
			eca-logger-interface.h \
			eca-comhelp.h \
			eca-engine.h \
			eca-engine-driver.h \
			eca-engine_impl.h \
			eca-session.h \
			eca-resources.h \
			resource-file.h \
			generic-controller.h \
			layer.h \
			eca-static-object-maps.h \
			eca-object-map.h \
			eca-preset-map.h \
			samplebuffer.h \
			samplebuffer_impl.h \
			samplebuffer_functions.h \
			samplebuffer_iterators.h \
			sample-specs.h \
			eca-version.h \
			eca-object-factory.h \
			eca-chainsetup.h \
			eca-chainsetup_impl.h \
			eca-chainsetup-bufparams.h \
			eca-chainsetup-parser.h \
			eca-chainsetup-position.h \
			eca-control.h \
			eca-control-base.h \
			eca-control-objects.h \
			eca-control-dump.h \
			eca-iamode-parser.h \
			eca-control-interface.h \
			eca-audio-position.h \
			eca-fileio.h  \
			eca-fileio-stream.h \
			eca-fileio-mmap.h \
			dynamic-parameters.h \
			dynamic-object.h \
			eca-object.h \
			eca-operator.h \
			eca-samplerate-aware.h \
			eca-audio-format.h \
			eca-audio-time.h \
			preset.h \
			preset_impl.h \
			file-preset.h \
			global-preset.h \
			midiio.h

ecasound_extra_include = \
			ladspa.h

ecasound_test_framework_include = \
			eca-test-repository.h \
			eca-test-case.h \
			eca-chainsetup_test.h \
			eca-object-factory_test.h \
			audioio_test.h \
			audioio-device_test.h

# VST-support:
#			eca-vst-plugin-map.h
# vst_header_files = AEffEditor.hpp AEffect.h aeffectx.h AudioEffect.hpp
#			$(vst_header_files)
#			audiofx_vst.h

noinst_HEADERS = 	$(ecasound_audiofx_include) \
			$(ecasound_audioio_include) \
			$(ecasound_midi_include) \
		 	$(ecasound_controller_include) \
			$(ecasound_general_include) \
			$(ecasound_extra_include) \
			$(ecasound_test_framework_include)

# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

# vst_src_files = AudioEffect.cpp audioeffectx.cpp
# 			audiofx_vst.cpp \
# 			eca-vst-plugin-map.cpp \
# $(vst_src_files)

ecasound_audiofx_src =  audiofx.cpp \
			audiofx_misc.cpp \
			audiofx_amplitude.cpp \
			audiofx_compressor.cpp \
			audiofx_analysis.cpp \
			audiofx_envelope_modulation.cpp \
			audiofx_filter.cpp \
			audiofx_rcfilter.cpp \
			audiofx_reverb.cpp \
			audiofx_timebased.cpp \
			audiogate.cpp \
			audiofx_mixing.cpp \
			audiofx_ladspa.cpp \
			audio-stamp.cpp


if ECA_AM_COMPILE_OSS
ecasound_audioio_oss_src = audioio-oss.cpp
else
ecasound_audioio_oss_src = 
endif

ecasound_audioio_src = 	audioio-cdr.cpp \
			audioio-ewf.cpp \
			audioio-mp3.cpp \
			audioio-ogg.cpp \
			audioio-wave.cpp \
			audioio.cpp \
			audioio-buffered.cpp \
			audioio-device.cpp \
			audioio-raw.cpp \
			audioio-mikmod.cpp \
			audioio-rtnull.cpp \
			audioio-loop.cpp \
			audioio-forked-stream.cpp \
			audioio-timidity.cpp \
			audioio-proxy-server.cpp \
			audioio-proxy-buffer.cpp \
			audioio-buffered-proxy.cpp \
			audioio-typeselect.cpp \
			audioio-reverse.cpp \
			$(ecasound_audioio_oss_src)

ecasound_midi_src =	\
			midi-server.cpp \
			midi-client.cpp \
			midi-parser.cpp \
			midiio.cpp \
			midiio-raw.cpp

ecasound_controller_src = \
			generic-controller.cpp \
			midi-cc.cpp \
			osc-gen.cpp \
			osc-gen-file.cpp \
			osc-sine.cpp \
			linear-envelope.cpp \
			two-stage-linear-envelope.cpp \
			stamp-ctrl.cpp \
			generic-linear-envelope.cpp

ecasound_general_src = 	eca-chain.cpp \
			eca-engine.cpp \
			samplebuffer.cpp \
			eca-session.cpp \
			eca-resources.cpp \
			resource-file.cpp \
			eca-comhelp.cpp \
			eca-logger.cpp \
			layer.cpp \
			samplebuffer_iterators.cpp \
			eca-version.cpp  \
			eca-operator.cpp \
			eca-object-factory.cpp \
			eca-chainsetup.cpp \
			eca-chainsetup-bufparams.cpp \
			eca-chainsetup-parser.cpp \
			eca-chainsetup-position.cpp \
			eca-control.cpp \
			eca-control-base.cpp \
			eca-control-objects.cpp \
			eca-control-dump.cpp \
			eca-iamode-parser.cpp \
			eca-control-interface.cpp \
			eca-samplerate-aware.cpp \
			eca-audio-position.cpp \
			eca-audio-format.cpp \
			eca-audio-time.cpp \
			eca-fileio-stream.cpp \
			eca-fileio-mmap.cpp \
			eca-static-object-maps.cpp \
			preset.cpp \
			file-preset.cpp \
			eca-object-map.cpp \
			eca-preset-map.cpp \
			global-preset.cpp

ecasound_common_src = 	$(ecasound_audiofx_src) \
			$(ecasound_audioio_src) \
			$(ecasound_midi_src) \
		      	$(ecasound_controller_src) \
			$(ecasound_general_src)

libecasound_tester_src = \
			libecasound_tester.cpp \
			eca-test-repository.cpp \
			eca-test-case.cpp

libecasound8_la_SOURCES = $(ecasound_common_src)
libecasound8_debug_la_SOURCES = $(ecasound_common_src)
libecasound8_la_CFLAGS = $(eca_cxxflags)
libecasound8_la_LIBADD = $(eca_libadd)
libecasound8_la_LDFLAGS = -export-dynamic $(eca_ldflags)
libecasound8_debug_la_CFLAGS = $(libecasound8_la_CFLAGS)
libecasound8_debug_la_LDFLAGS = $(libecasound8_la_LDFLAGS)
libecasound8_debug_la_LIBADD = $(libecasound8_la_LIBADD)

libecasound_tester_SOURCES = $(libecasound_tester_src)
libecasound_tester_CFLAGS = $(eca_cxxflags)
libecasound_tester_LDADD = $(libecasound_tester_libs)

# ---------------------------------------------------------------------
# Install targets - note! we don't install $ecasound_extra_include
#                         nor $ecasound_test_framework_include
install-data-local:
	$(INSTALL) -d $(DESTDIR)$(includedir)/libecasound@LIBECASOUND_VERSION@
	cd $(srcdir) ; \
	$(INSTALL_DATA) \
		$(ecasound_audiofx_include) \
		$(ecasound_audioio_include) \
		$(ecasound_controller_include) \
		$(ecasound_midi_include) \
		$(ecasound_general_include) \
		$(DESTDIR)$(includedir)/libecasound@LIBECASOUND_VERSION@
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) libecasound-config $(DESTDIR)$(bindir)/libecasound@LIBECASOUND_VERSION@-config

# ---------------------------------------------------------------------
# Uninstall targets
uninstall-local:
	cd $(DESTDIR)$(includedir)/libecasound@LIBECASOUND_VERSION@ && \
	rm -f   $(ecasound_audiofx_include) \
		$(ecasound_audioio_include) \
		$(ecasound_controller_include) \
		$(ecasound_midi_include) \
		$(ecasound_general_include)
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(includedir)/libecasound@LIBECASOUND_VERSION@
	rm -f 	$(DESTDIR)$(bindir)/libecasound@LIBECASOUND_VERSION@-config
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(bindir)
