AUTOMAKE_OPTIONS 		= foreign
EXTRA_DIST 			= eci.py pyeca.py

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if ECA_AM_DEBUG_MODE
ecasoundc_libs 			= -L$(top_builddir)/libecasoundc/.libs -lecasoundc_debug
else
ecasoundc_libs 			= -L$(top_builddir)/libecasoundc/.libs -lecasoundc
endif

# difficult from RPM point-of-view
# libdir = $(ECA_S_PYTHON_DLMODULES)
# datadir = $(ECA_S_PYTHON_MODULES)
# libdir = $(prefix)/lib/site-lisp

INCLUDES 			= -I$(srcdir) \
				-I$(top_srcdir)/libecasoundc \
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

libpyecasound_la_SOURCES	= pyecasound.c
# old LDFLAGS: -Wl,--rpath -Wl,${prefix}/lib
libpyecasound_la_LDFLAGS 	= -avoid-version -export-dynamic

if ECA_AM_COMPILE_PYECASOUND
all: pyecasound.so

pyecasound.so: pyecasound.o
	$(CC) -o pyecasound.so pyecasound.o -shared -nostartfiles $(libpyecasound_la_LDFLAGS) $(ecasoundc_libs)
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
	$(INSTALL) pyecasound.so $(srcdir)/pyeca.py $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
else
install-exec-hook:
endif

install-data-hook:
if ECA_AM_COMPILE_PYECASOUND
	$(INSTALL) -d $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
	cd $(srcdir) && cp eci.py $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
endif

uninstall-local:
if ECA_AM_COMPILE_PYECASOUND
	rm -f   $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/eci.py
	rm -f   $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/pyeca.py
	rm -f 	$(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/pyecasound.so
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
endif
