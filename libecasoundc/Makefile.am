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

INCLUDES = -I$(srcdir)
# -I$(top_srcdir)/libecasound

# ----------------------------------------------------------------------
# build targets and compiler options target defines
# ----------------------------------------------------------------------

if ECA_AM_DEBUG_MODE
lib_LTLIBRARIES = libecasoundc_debug.la
else
lib_LTLIBRARIES = libecasoundc.la
endif

TESTS = libecasoundc_tester

check_PROGRAMS = libecasoundc_tester

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

ecasoundc_includes = ecasoundc.h
noinst_HEADERS = $(ecasoundc_includes)

# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

# implementation using libecasound
#ecasoundc_src = ecasoundc.cpp

# standalone implementation (forks and execs the ecasound binary)
ecasoundc_src = ecasoundc_sa.c

# ----------------------------------------------------------------------
# library definitons
# ----------------------------------------------------------------------

libecasoundc_la_SOURCES = $(ecasoundc_src)
libecasoundc_la_LIBADD =
libecasoundc_la_LDFLAGS = -export-dynamic $(eca_ldflags)
libecasoundc_debug_la_SOURCES = $(libecasoundc_la_SOURCES)
libecasoundc_debug_la_LIBADD =
libecasoundc_debug_la_LDFLAGS = $(libecasoundc_la_LDFLAGS)

libecasoundc_tester_SOURCES = libecasoundc_tester.c ecasoundc_sa.c
libecasoundc_tester_LDADD = 

# ---------------------------------------------------------------------
# Install targets
install-data-local:
	$(INSTALL) -d $(DESTDIR)$(includedir)/libecasoundc
	cd $(srcdir) ; \
	$(INSTALL_DATA) $(ecasoundc_includes) \
		$(DESTDIR)$(includedir)/libecasoundc
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) libecasoundc-config $(DESTDIR)$(bindir)/

# ---------------------------------------------------------------------
# Uninstall targets
uninstall-local:
	cd $(DESTDIR)$(includedir)/libecasoundc && \
	rm -f   $(ecasoundc_includes)
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(includedir)/libecasoundc
	rm -f 	$(DESTDIR)$(bindir)/libecasoundc-config
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(bindir)
