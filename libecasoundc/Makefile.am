EXTRA_DIST = ChangeLog
AUTOMAKE_OPTIONS = foreign

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

# !!!
# remember to update eca-version.cpp
if ECA_AM_DEBUG_MODE
eca_ldflags = -version-info @LIBECASOUNDC_VERSION@:0:0
else
eca_ldflags = -s -version-info @LIBECASOUNDC_VERSION@:0:0
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir)/libecasound

if ECA_AM_DEBUG_MODE
lib_LTLIBRARIES = libecasoundc2_debug.la
else
lib_LTLIBRARIES = libecasoundc2.la
endif

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

ecasoundc_includes = ecasoundc.h
noinst_HEADERS = $(ecasoundc_includes)

# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

ecasoundc_src = ecasoundc.cpp

# ----------------------------------------------------------------------
# library definitons
# ----------------------------------------------------------------------

libecasoundc2_la_SOURCES = $(ecasoundc_src)
libecasoundc2_la_LIBADD = -lstdc++
libecasoundc2_la_LDFLAGS = -export-dynamic $(eca_ldflags)
libecasoundc2_debug_la_SOURCES = $(libecasoundc2_la_SOURCES)
libecasoundc2_debug_la_LIBADD = -lstdc++
libecasoundc2_debug_la_LDFLAGS = $(libecasoundc2_la_LDFLAGS)

# ---------------------------------------------------------------------
# Install targets
install-data-local:
	$(INSTALL) -d $(DESTDIR)$(includedir)/libecasoundc@LIBECASOUNDC_VERSION@
	cd $(srcdir) ; \
	$(INSTALL_DATA) $(ecasoundc_includes) \
		$(DESTDIR)$(includedir)/libecasoundc@LIBECASOUNDC_VERSION@
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) libecasoundc-config $(DESTDIR)$(bindir)/libecasoundc@LIBECASOUNDC_VERSION@-config

# ---------------------------------------------------------------------
# Uninstall targets
uninstall-local:
	cd $(DESTDIR)$(includedir)/libecasoundc@LIBECASOUNDC_VERSION@ && \
	rm -f   $(ecasoundc_includes)
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(includedir)/libecasoundc@LIBECASOUNDC_VERSION@
	rm -f 	$(DESTDIR)$(bindir)/libecasoundc@LIBECASOUNDC_VERSION@-config
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(bindir)
