EXTRA_DIST = BUGS NEWS TODO FAQ generic_oscillators effect_presets ecasound.spec

if SYSTEM_READLINE
SUBDIRS = kvutils libecasound libecasoundc pyecasound ecasound ecatools Documentation
else
SUBDIRS = kvutils readline-4.0 libecasound libecasoundc pyecasound ecasound ecatools Documentation
endif
AUTOMAKE_OPTIONS = foreign

PACKAGE = @PACKAGE@
VERSION = @VERSION@
RELEASE = 1

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(datadir)/ecasound
	if [ ! -f $(DESTDIR)$(datadir)/ecasound/generic_oscillators ]; then $(INSTALL_DATA) generic_oscillators $(DESTDIR)$(datadir)/ecasound/generic_oscillators ; fi
	if [ ! -f $(DESTDIR)$(datadir)/ecasound/effect_presets ]; then $(INSTALL_DATA) effect_presets $(DESTDIR)$(datadir)/ecasound/effect_presets; fi

delete-static-libs:
	rm -f $(DESTDIR)$(prefix)/lib/libkvutils.a
	rm -f $(DESTDIR)$(prefix)/lib/libecasound.a

strip-shared-libs:
	strip $(DESTDIR)$(prefix)/lib/libecasound.so
	strip $(DESTDIR)$(prefix)/lib/libkvutils.so

redhat-rpm: ecasound.spec dist
	cp -fv $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
	rpm -ba ecasound.spec

dist-build-rpm: ecasound.spec dist
	cp -fv $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
	rm -Rf /tmp/ecasound-build
	rpm -ba ecasound.spec
	rpm --addsign /usr/src/redhat/SRPMS/$(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm
	rpm --addsign /usr/src/redhat/RPMS/i386/$(PACKAGE)*$(VERSION)-$(RELEASE).i386.rpm

move-to-seul:
	mv -v /usr/src/redhat/SOURCES/$(PACKAGE)-$(VERSION).tar.gz /opt/http/ecasound.seul/
	rm -f $(PACKAGE)-$(VERSION).tar.gz
	mv -v /usr/src/redhat/RPMS/i386/$(PACKAGE)-$(VERSION)-$(RELEASE).i386.rpm /opt/http/ecasound.seul/
	mv -v /usr/src/redhat/RPMS/i386/$(PACKAGE)-plugins-$(VERSION)-$(RELEASE).i386.rpm /opt/http/ecasound.seul/
	mv -v /usr/src/redhat/RPMS/i386/$(PACKAGE)-devel-$(VERSION)-$(RELEASE).i386.rpm /opt/http/ecasound.seul/
	mv -v /usr/src/redhat/SRPMS/$(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm /opt/http/ecasound.seul/

