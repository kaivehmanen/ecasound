EXTRA_DIST = ChangeLog
AUTOMAKE_OPTIONS = foreign

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

# !!!
# remember to update eca-version.cpp
if INCLUDE_DEBUG
eca_ldflags = -version-info 0:0:0
ecasound_libs = -lecasound_debug
else
eca_ldflags = -s -version-info 0:0:0
ecasound_libs = -lecasound
endif

if KVUTILS_INSTALLED
kvutils_libs = -lkvutils
else
kvutils_libs = -L$(top_srcdir)/kvutils/.libs -lkvutils
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
libecasoundc_debug_la_SOURCES = $(libecasoundc_la_SOURCES)
libecasoundc_la_LIBADD = $(ecasound_libs) $(kvutils_libs)
libecasoundc_debug_la_LIBADD = $(libecasoundc_la_LIBADD)
libecasoundc_la_LDFLAGS = -export-dynamic $(eca_ldflags)
libecasoundc_debug_la_LDFLAGS = $(libecasoundc_la_LDFLAGS)

# ----------------------------------------------------------------------
# hooks
# ----------------------------------------------------------------------

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(pkgincludedir)
	cd $(srcdir) ; \
	cp      $(ecasoundc_includes) \
		$(DESTDIR)$(pkgincludedir)
