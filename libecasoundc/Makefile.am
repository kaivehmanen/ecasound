EXTRA_DIST = ChangeLog
AUTOMAKE_OPTIONS = foreign

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

# !!!
# remember to update eca-version.cpp
if INCLUDE_DEBUG
eca_ldflags = -version-info 1:0:1
else
eca_ldflags = -s -version-info 1:0:1
endif

if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir) -I$(top_srcdir)/libecasound

if INCLUDE_DEBUG
lib_LTLIBRARIES = libecasoundc_debug.la
else
lib_LTLIBRARIES = libecasoundc.la
endif

bin_SCRIPTS = ecasoundc-config

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

libecasoundc_la_SOURCES = $(ecasoundc_src)
libecasoundc_la_LIBADD = -lstdc++
libecasoundc_la_LDFLAGS = -export-dynamic $(eca_ldflags)
libecasoundc_debug_la_SOURCES = $(libecasoundc_la_SOURCES)
libecasoundc_debug_la_LIBADD = -lstdc++
libecasoundc_debug_la_LDFLAGS = $(libecasoundc_la_LDFLAGS)

# ---------------------------------------------------------------------
# Install targets
install-data-local:
	$(INSTALL) -d $(DESTDIR)$(pkgincludedir)
	cd $(srcdir) ; \
	cp      $(ecasoundc_includes) \
		$(DESTDIR)$(pkgincludedir)

# ---------------------------------------------------------------------
# Uninstall targets
uninstall-local:
	cd $(DESTDIR)$(pkgincludedir) ; \
	rm -f   $(ecasoundc_includes)
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(pkgincludedir)