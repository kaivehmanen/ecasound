ecasound_audiofx_src =  audiofx.h \
			audiofx_impl.h \
			audiofx_amplitude.h \
			audiofx_compressor.h \
			audiofx_analysis.h \
			audiofx_filter.h \
			audiofx_rcfilter.h \
			audiofx_timebased.h \
			audiogate.h \
			audiofx_mixing.h

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
			eca-chainop-map.h \
			eca-controller-map.h \
			eca-audio-object-map.h \
			eca-preset-map.h \
			samplebuffer.h \
			sample-specs.h \
			eca-version.h \
			eca-chainsetup.h \
			eca-controller.h \
			eca-controller-base.h \
			eca-controller-objects.h \
			eca-iamode-parser.h \
			osc-gen.h \
			midi-cc.h \
			eca-control-position.h \
			eca-audio-position.h \
			eca-fileio.h  \
			eca-fileio-stream.h \
			eca-fileio-mmap.h \
			eca-fileio-mmap-fthread.h \
			eca-alsa-dyn.h \
			eca-mthreaded-processor.h \
			dynamic-parameters.h \
			dynamic-object.h \
			samplebuffer_iterators.h \
			eca-audio-objects.h \
			eca-audio-format.h \
			eca-audio-time.h \
			preset.h \
			file-preset.h \
			global-preset.h

#pkgincludedir_HEADERS = $(ecasound_audiofx_src) $(ecasound_audioio_src) $(ecasound_general_src)

noinst_HEADERS = $(ecasound_audiofx_src) $(ecasound_audioio_src) \
		 $(ecasound_controller_src) $(ecasound_general_src) 

install-data-local:
	$(INSTALL) -d $(pkgincludedir)
	cd $(srcdir) ; \
	cp $(ecasound_audiofx_src) $(ecasound_audioio_src) $(ecasound_controller_src) $(ecasound_general_src) \
	$(pkgincludedir)
