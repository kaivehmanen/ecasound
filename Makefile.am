EXTRA_DIST = BUGS COPYING.GPL COPYING.LGPL NEWS TODO generic_oscillators effect_presets ecasound.spec

# for automake 1.4/1.5 compatibility
EXTRA_DIST += depcomp

if ECA_AM_SYSTEM_READLINE
SUBDIRS = kvutils libecasound ecasound libecasoundc pyecasound ecatools Documentation examples
else
SUBDIRS = kvutils readline-4.0 libecasound ecasound libecasoundc pyecasound ecatools Documentation examples
endif
AUTOMAKE_OPTIONS = foreign

PACKAGE = @PACKAGE@
VERSION = @VERSION@
RELEASE = 1

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(datadir)/ecasound
	$(INSTALL_DATA) $(top_builddir)/ecasoundrc $(DESTDIR)$(datadir)/ecasound/ecasoundrc
	$(INSTALL_DATA) $(top_srcdir)/generic_oscillators $(DESTDIR)$(datadir)/ecasound/generic_oscillators
	$(INSTALL_DATA) $(top_srcdir)/effect_presets $(DESTDIR)$(datadir)/ecasound/effect_presets

uninstall-local:
	rm -f	$(DESTDIR)$(datadir)/ecasound/generic_oscillators \
		$(DESTDIR)$(datadir)/ecasound/effect_presets \
		$(DESTDIR)$(datadir)/ecasound/ecasoundrc
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(datadir)/ecasound
