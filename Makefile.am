EXTRA_DIST = BUGS NEWS FAQ generic_oscillators effect_presets ecasound.spec

SUBDIRS = kvutils readline-4.0 libecasound ecasound libqtecasound qtecasound ecatools Documentation
AUTOMAKE_OPTIONS = foreign

PACKAGE = @PACKAGE@
VERSION = @VERSION@
RELEASE = 1

install-data-local:
	$(INSTALL) -d $(datadir)/ecasound
	if [ ! -f $(datadir)/ecasound/generic_oscillators ]; then $(INSTALL_DATA) generic_oscillators $(datadir)/ecasound/generic_oscillators ; fi
	if [ ! -f $(datadir)/ecasound/effect_presets ]; then $(INSTALL_DATA) effect_presets $(datadir)/ecasound/effect_presets; fi
#	if [ ! -f $(datadir)/ecasound/singlechain_effect_presets ]; then $(INSTALL_DATA) singlechain_effect_presets $(datadir)/ecasound/singlechain_effect_presets; fi
#	if [ ! -f $(datadir)/ecasound/multichain_effect_presets ]; then $(INSTALL_DATA) multichain_effect_presets $(datadir)/ecasound/multichain_effect_presets ; fi

delete-static-libs:
	rm -f $(libdir)/libkvutils.a
	rm -f $(libdir)/libecasound.a
	rm -f $(libdir)/libqtecasound.a

strip-shared-libs:
	strip $(libdir)/libecasound.so
	strip $(libdir)/libqtecasound.so
	strip $(libdir)/libkvutils.so

build-rpms: ecasound.spec dist
	cp -fv $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
	rm -Rf /tmp/ecasound-build
	export extra_params=--disable-alsa ; rpm -ba ecasound.spec
	rpm --addsign /usr/src/redhat/SRPMS/$(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm
	rpm --addsign /usr/src/redhat/RPMS/i386/$(PACKAGE)*$(VERSION)-$(RELEASE).i386.rpm

copy-to-seul:
	cp -v /usr/src/redhat/SOURCES/$(PACKAGE)-$(VERSION).tar.gz /opt/http/ecasound.seul/
	cp -v /usr/src/redhat/RPMS/i386/$(PACKAGE)-$(VERSION)-$(RELEASE).i386.rpm /opt/http/ecasound.seul/
	cp -v /usr/src/redhat/RPMS/i386/$(PACKAGE)-devel-$(VERSION)-$(RELEASE).i386.rpm /opt/http/ecasound.seul/
	cp -v /usr/src/redhat/RPMS/i386/$(PACKAGE)-qt-$(VERSION)-$(RELEASE).i386.rpm /opt/http/ecasound.seul/
	cp -v /usr/src/redhat/SRPMS/$(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm /opt/http/ecasound.seul/
