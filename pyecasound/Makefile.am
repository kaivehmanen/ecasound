AUTOMAKE_OPTIONS 		= foreign
EXTRA_DIST 			= pyeca.py

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if INCLUDE_DEBUG
ecasound_libs 			= -L$(top_builddir)/libecasound/.libs -lecasound_debug
else
ecasound_libs 			= -L$(top_builddir)/libecasound/.libs -lecasound
endif

if KVUTILS_INSTALLED
kvutils_libs 			= -lkvutils
else
kvutils_libs 			= -L$(top_builddir)/kvutils/.libs -lkvutils
endif

# difficult from RPM point-of-view
libdir = $(eca_python_dlmodules)
datadir = $(eca_python_modules)
# libdir = $(prefix)/lib/site-lisp

INCLUDES 			= -I$(srcdir) -I$(top_srcdir) \
				-I$(top_srcdir)/libecasound \
				-I$(eca_python_incdir)

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

pyecasound_includes 		= pyecasound.h
noinst_HEADERS 			= $(pyecasound_includes)

# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

if COMPILE_PYECASOUND
lib_LTLIBRARIES 		= libpyecasound.la
endif

libpyecasound_la_SOURCES	= pyecasound.cpp

libpyecasound_la_LIBADD		= $(ecasound_libs) $(kvutils_libs)
libpyecasound_la_LDFLAGS 	= -avoid-version -export-dynamic

# ----------------------------------------------------------------------
# hooks
# ----------------------------------------------------------------------

install-data-hook:
if COMPILE_PYECASOUND
	$(INSTALL) -d $(DESTDIR)$(libdir)
	cd $(srcdir) ; cp pyeca.py $(DESTDIR)$(libdir)
endif
