EXTRA_DIST = BUGS NEWS COPYING.Qt ecawave.spec
SUBDIRS = src Documentation
AUTOMAKE_OPTIONS = foreign
RELEASE = 1

create-rpm-spec:
	cat $(PACKAGE).spec | awk -- '{ print gensub("METAVERSION","$(VERSION)",g) }' >$(PACKAGE)-$(VERSION)-$(RELEASE).spec

redhat-rpm: create-rpm-spec dist
	cp -fv $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
	rpm -ba $(PACKAGE)-$(VERSION)-$(RELEASE).spec

dist-build-rpm: create-rpm-spec dist
	cp -fv $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
	rm -Rf /tmp/ecawave-build
	rpm -ba $(PACKAGE)-$(VERSION)-$(RELEASE).spec
	rpm --addsign /usr/src/redhat/SRPMS/$(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm
	rpm --addsign /usr/src/redhat/RPMS/i386/$(PACKAGE)*$(VERSION)-$(RELEASE).i386.rpm

copy-to-seul:
	cp -v /usr/src/redhat/SOURCES/$(PACKAGE)-$(VERSION).tar.gz /opt/http/ecasound.seul/
	cp -v /usr/src/redhat/RPMS/i386/$(PACKAGE)-$(VERSION)-$(RELEASE).i386.rpm /opt/http/ecasound.seul/
	cp -v /usr/src/redhat/SRPMS/$(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm /opt/http/ecasound.seul/
