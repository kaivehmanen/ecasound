EXTRA_DIST = BUGS NEWS Documentation
SUBDIRS = src Documentation
AUTOMAKE_OPTIONS = foreign

create-rpm-spec:
	cat $(PACKAGE).spec | awk -- '{ print gensub("METAVERSION","$(VERSION)",g) }' >$(PACKAGE)-$(VERSION)-$(RELEASE).spec

build-rpm: create-rpm-spec $(PACKAGE)-$(VERSION).tar.gz
	cp -fv $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES/
	rm -Rf /tmp/ecawave-build
	rpm -ba $(PACKAGE)-$(VERSION)-$(RELEASE).spec
	rpm --addsign /usr/src/redhat/SRPMS/$(PACKAGE)-$(VERSION)-$(RELEASE).src.rpm
	rpm --addsign /usr/src/redhat/RPMS/i386/$(PACKAGE)*$(VERSION)-$(RELEASE).i386.rpm

copy-to-seul:
	cp -v /usr/src/redhat/SOURCES/$(PACKAGE)-$(VERSION).tar.gz /opt/http/ecasound.seul/
	cp -v /usr/src/redhat/RPMS/i386/$(PACKAGE)-$(VERSION)-$(RELEASE).i386.rpm /opt/http/ecasound.seul/
