AUTOMAKE_OPTIONS 		= foreign
EXTRA_DIST 			= eci.py

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if INCLUDE_DEBUG
ecasound_libs 			= -L$(top_builddir)/libecasound/.libs -lecasound_debug
else
ecasound_libs 			= -L$(top_builddir)/libecasound/.libs -lecasound
endif

if INCLUDE_DEBUG
kvutils_libs                    = -L$(top_builddir)/kvutils/.libs -lkvutils_debug
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
endif

libpyecasound_la_SOURCES	= pyecasound.cpp
libpyecasound_la_LIBADD		= $(ecasound_libs) $(kvutils_libs)
libpyecasound_la_LDFLAGS 	= -avoid-version -export-dynamic -Wl,--rpath -Wl,${prefix}/lib

if COMPILE_PYECASOUND
all: pyeca.so

pyeca.so: pyecasound.o
	$(CXX) -o pyeca.so pyecasound.o -shared -nostartfiles  $(libpyecasound_la_LDFLAGS) $(ecasound_libs) $(kvutils_libs)
endif

# ----------------------------------------------------------------------
# hooks
# ----------------------------------------------------------------------

if COMPILE_PYECASOUND
CLEANFILES=pyeca.so
endif

if COMPILE_PYECASOUND
install-exec-hook: pyeca.so
	$(INSTALL) pyeca.so $(DESTDIR)$(eca_python_dlmodules)/pyeca.so
else
install-exec-hook:
endif

install-data-hook:
if COMPILE_PYECASOUND
	$(INSTALL) -d $(DESTDIR)$(eca_python_dlmodules)
	cd $(srcdir) ; cp eci.py $(DESTDIR)$(eca_python_dlmodules)
endif

uninstall-local:
if COMPILE_PYECASOUND
	rm -f   $(DESTDIR)$(eca_python_dlmodules)/eci.py
	rm -f 	$(DESTDIR)$(eca_python_dlmodules)/pyeca.so
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(eca_python_dlmodules)
endif