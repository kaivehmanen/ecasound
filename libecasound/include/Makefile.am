ecasound_audiofx_src =  audiofx.h \
			audiofx_impl.h \
			audiofx_amplitude.h \
			audiofx_compressor.h \
			audiofx_analysis.h \
			audiofx_filter.h \
			audiofx_rcfilter.h \
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
			resource-file.h\
			generic-controller.h \
			layer.h \
			eca-static-object-maps.h \
			eca-object-map.h \
			eca-preset-map.h \
			eca-ladspa-plugin-map.h \
			eca-vst-plugin-map.h
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

#			eca-chainop-map.h \
#			eca-controller-map.h \
#			eca-audio-object-map.h \

#pkgincludedir_HEADERS = $(ecasound_audiofx_src) $(ecasound_audioio_src) $(ecasound_general_src)

if FEELING_EXPERIMENTAL
vst_files = AEffEditor.hpp AEffect.h aeffectx.h AudioEffect.hpp
endif

noinst_HEADERS = $(ecasound_audiofx_src) $(ecasound_audioio_src) \
		 $(ecasound_controller_src) $(ecasound_general_src) $(vst_files)

install-data-local:
	$(INSTALL) -d $(pkgincludedir)
	cd $(srcdir) ; \
	cp 	$(ecasound_audiofx_src) \
		$(ecasound_audioio_src) \
		$(ecasound_controller_src) \
		$(ecasound_general_src) $(vst_files) \
		$(pkgincludedir)
