AUTOMAKE_OPTIONS 		= foreign
EXTRA_DIST 			= eci.py pyeca.py

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if ECA_AM_DEBUG_MODE
ecasound_libs 			= -L$(top_builddir)/libecasound/.libs -lecasound_debug
else
ecasound_libs 			= -L$(top_builddir)/libecasound/.libs -lecasound
endif

if ECA_AM_DEBUG_MODE
kvutils_libs                    = -L$(top_builddir)/kvutils/.libs -lkvutils_debug
else
kvutils_libs 			= -L$(top_builddir)/kvutils/.libs -lkvutils
endif

# difficult from RPM point-of-view
# libdir = $(ECA_S_PYTHON_DLMODULES)
# datadir = $(ECA_S_PYTHON_MODULES)
# libdir = $(prefix)/lib/site-lisp

INCLUDES 			= -I$(srcdir) -I$(top_srcdir) \
				-I$(top_srcdir)/libecasound \
				-I$(ECA_S_PYTHON_INCLUDES)

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

pyecasound_includes 		= pyecasound.h
noinst_HEADERS 			= $(pyecasound_includes)

# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

if ECA_AM_COMPILE_PYECASOUND
noinst_LTLIBRARIES 		= libpyecasound.la
endif

libpyecasound_la_SOURCES	= pyecasound.cpp
libpyecasound_la_LIBADD		= $(ecasound_libs) $(kvutils_libs)
libpyecasound_la_LDFLAGS 	= -avoid-version -export-dynamic -Wl,--rpath -Wl,${prefix}/lib

if ECA_AM_COMPILE_PYECASOUND
all: pyecasound.so

pyecasound.so: pyecasound.o
	$(CXX) -o pyecasound.so pyecasound.o -shared -nostartfiles  $(libpyecasound_la_LDFLAGS) $(ecasound_libs) $(kvutils_libs)
endif

# ----------------------------------------------------------------------
# hooks
# ----------------------------------------------------------------------

if ECA_AM_COMPILE_PYECASOUND
CLEANFILES=pyecasound.so
endif

if ECA_AM_COMPILE_PYECASOUND
install-exec-hook: pyecasound.so
	$(INSTALL) -d $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
	$(INSTALL) pyecasound.so pyeca.py $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
else
install-exec-hook:
endif

install-data-hook:
if ECA_AM_COMPILE_PYECASOUND
	$(INSTALL) -d $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
	cd $(srcdir) ; cp eci.py $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
endif

uninstall-local:
if ECA_AM_COMPILE_PYECASOUND
	rm -f   $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/eci.py
	rm -f   $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/pyeca.py
	rm -f 	$(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/pyecasound.so
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
endif