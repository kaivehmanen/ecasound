EXTRA_DIST = BUGS NEWS FAQ generic_oscillators multichain_effect_presets singlechain_effect_presets

SUBDIRS = kvutils readline-4.0 libecasound ecasound libqtecasound qtecasound ecatools Documentation
AUTOMAKE_OPTIONS = foreign

install-data-local:
	$(INSTALL) -d $(datadir)/ecasound
	if [ ! -f $(datadir)/ecasound/singlechain_effect_presets ]; then $(INSTALL_DATA) singlechain_effect_presets $(datadir)/ecasound/singlechain_effect_presets; fi
	if [ ! -f $(datadir)/ecasound/multichain_effect_presets ]; then $(INSTALL_DATA) multichain_effect_presets $(datadir)/ecasound/multichain_effect_presets ; fi
	if [ ! -f $(datadir)/ecasound/generic_oscillators ]; then $(INSTALL_DATA) generic_oscillators $(datadir)/ecasound/generic_oscillators ; fi

delete-static-libs:
	rm -f $(libdir)/libkvutils.a
	rm -f $(libdir)/libecasound.a
	rm -f $(libdir)/libqtecasound.a

strip-shared-libs:
	strip $(libdir)/libecasound.so
	strip $(libdir)/libqtecasound.so
	strip $(libdir)/libkvutils.so

