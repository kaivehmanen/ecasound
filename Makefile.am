EXTRA_DIST = BUGS NEWS TODO FAQ generic_oscillators effect_presets ecasound.spec ecasound.mdk.spec

if SYSTEM_READLINE
SUBDIRS = kvutils libecasound ecasound libqtecasound qtecasound ecatools Documentation
else
SUBDIRS = kvutils readline-4.0 libecasound ecasound libqtecasound qtecasound ecatools Documentation
endif
AUTOMAKE_OPTIONS = foreign

PACKAGE = @PACKAGE@
VERSION = @VERSION@
RELEASE = 1

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(datadir)/ecasound
	if [ ! -f $(DESTDIR)$(datadir)/ecasound/generic_oscillators ]; then $(INSTALL_DATA) generic_oscillators $(DESTDIR)$(datadir)/ecasound/generic_oscillators ; fi
	if [ ! -f $(DESTDIR)$(datadir)/ecasound/effect_presets ]; then $(INSTALL_DATA) effect_presets $(DESTDIR)$(datadir)/ecasound/effect_presets; fi
#	if [ ! -f $(datadir)/ecasound/singlechain_effect_presets ]; then $(INSTALL_DATA) singlechain_effect_presets $(datadir)/ecasound/singlechain_effect_presets; fi
#	if [ ! -f $(datadir)/ecasound/multichain_effect_presets ]; then $(INSTALL_DATA) multichain_effect_presets $(datadir)/ecasound/multichain_effect_presets ; fi

delete-static-libs:
	rm -f $(DESTDIR)$(libdir)/libkvutils.a
	rm -f $(DESTDIR)$(libdir)/libecasound.a
	rm -f $(DESTDIR)$(libdir)/libqtecasound.a

strip-shared-libs:
	strip $(DESTDIR)$(libdir)/libecasound.so
	strip $(DESTDIR)$(libdir)/libqtecasound.so
	strip $(DESTDIR)$(libdir)/libkvutils.so

redhat-rpm: ecasound.spec dist
	cp -fv $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
	rpm -ba ecasound.spec

dist-build-rpm: ecasound.spec dist
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
