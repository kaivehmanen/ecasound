AUTOMAKE_OPTIONS 		= foreign
EXTRA_DIST 			= pyeca.py eci.py

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if INCLUDE_DEBUG
ecasound_libs 			= -L$(top_builddir)/libecasound/.libs -lecasound_debug
else
ecasound_libs 			= -L$(top_builddir)/libecasound/.libs -lecasound
endif

if INCLUDE_DEBUG
kvutils_debug_libs              = -L$(top_builddir)/kvutils/.libs -lkvutils_debug
else
kvutils_libs 			= -L$(top_builddir)/kvutils/.libs -lkvutils
endif

# difficult from RPM point-of-view
# libdir = $(eca_python_dlmodules)
# datadir = $(eca_python_modules)
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
noinst_LTLIBRARIES 		= libpyecasound.la
#lib_LTLIBRARIES 		= libpyecasound.la
endif

libpyecasound_la_SOURCES	= pyecasound.cpp
libpyecasound_la_LIBADD		= $(ecasound_libs) $(kvutils_libs)
libpyecasound_la_LDFLAGS 	= -avoid-version -export-dynamic -Wl,--rpath -Wl,${prefix}/lib

if COMPILE_PYECASOUND
all: libpyecasound.so

libpyecasound.so: pyecasound.o
	$(CXX) -o libpyecasound.so pyecasound.o -shared -nostartfiles  $(libpyecasound_la_LDFLAGS) $(ecasound_libs) $(kvutils_libs)
endif

# ----------------------------------------------------------------------
# hooks
# ----------------------------------------------------------------------

if COMPILE_PYECASOUND
CLEANFILES=libpyecasound.so
endif

if COMPILE_PYECASOUND
install-exec-hook: libpyecasound.so
	$(INSTALL) libpyecasound.so $(DESTDIR)$(eca_python_dlmodules)/libpyecasound.so
else
install-exec-hook:
endif

install-data-hook:
if COMPILE_PYECASOUND
	$(INSTALL) -d $(DESTDIR)$(libdir)
	cd $(srcdir) ; cp pyeca.py eci.py $(DESTDIR)$(libdir)
endif

uninstall-local:
if COMPILE_PYECASOUND
	rm -f   $(DESTDIR)$(libdir)/pyeca.py \
		$(DESTDIR)$(libdir)/eci.py
	rm -f 	$(DESTDIR)$(eca_python_dlmodules)/libpyecasound.so
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(libdir)
endif