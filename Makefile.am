EXTRA_DIST = BUGS NEWS TODO generic_oscillators effect_presets ecasound.spec

# for automake 1.4/1.5 compatibility
EXTRA_DIST += depcomp

if SYSTEM_READLINE
SUBDIRS = kvutils libecasound libecasoundc pyecasound ecasound ecatools Documentation examples contrib
else
SUBDIRS = kvutils readline-4.0 libecasound libecasoundc pyecasound ecasound ecatools Documentation examples contrib
endif
AUTOMAKE_OPTIONS = foreign

PACKAGE = @PACKAGE@
VERSION = @VERSION@
RELEASE = 1

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(datadir)/ecasound
	if [ ! -f $(DESTDIR)$(datadir)/ecasound/generic_oscillators ]; then $(INSTALL_DATA) $(top_srcdir)/generic_oscillators $(DESTDIR)$(datadir)/ecasound/generic_oscillators ; fi
	if [ ! -f $(DESTDIR)$(datadir)/ecasound/effect_presets ]; then $(INSTALL_DATA) $(top_srcdir)/effect_presets $(DESTDIR)$(datadir)/ecasound/effect_presets; fi

uninstall-local:
	rm -f	$(DESTDIR)$(datadir)/ecasound/generic_oscillators \
		$(DESTDIR)$(datadir)/ecasound/effect_presets
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(datadir)/ecasound

delete-static-libs:
	rm -f $(DESTDIR)$(prefix)/lib/libkvutils.a
	rm -f $(DESTDIR)$(prefix)/lib/libecasound.a

strip-shared-libs:
	strip $(DESTDIR)$(prefix)/lib/libecasound.so
	strip $(DESTDIR)$(prefix)/lib/libkvutils.so
