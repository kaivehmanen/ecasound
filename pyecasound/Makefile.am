AUTOMAKE_OPTIONS 		= foreign
EXTRA_DIST 			= eci.py pyeca.py ecacontrol.py

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if ECA_AM_DEBUG_MODE
ecasoundc_libs 			= -L$(top_builddir)/libecasoundc/.libs -lecasoundc_debug
else
ecasoundc_libs 			= -L$(top_builddir)/libecasoundc/.libs -lecasoundc
endif

if ECA_AM_HAVE_PYTHON
pyecasound_install_list1 = 	$(srcdir)/eci.py \
				$(srcdir)/pyeca.py \
				$(srcdir)/ecacontrol.py
pyecasound_uninstall_list1 =    $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/eci.py \
				$(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/pyeca.py \
				$(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/ecacontrol.py
endif
if ECA_AM_COMPILE_PYECASOUND
pyecasound_install_list2 = 	pyecasound.so
pyecasound_uninstall_list2 =   	$(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/pyecasound.so
endif

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
libpyecasound_la_LDFLAGS 	= -avoid-version -export-dynamic

if ECA_AM_COMPILE_PYECASOUND
all: pyecasound.so

pyecasound.so: pyecasound.lo
	$(CC) -o pyecasound.so -shared -nostartfiles $(libpyecasound_la_LDFLAGS) pyecasound.lo $(top_builddir)/libecasoundc/ecasoundc_sa.lo
endif

# ----------------------------------------------------------------------
# hooks
# ----------------------------------------------------------------------

if ECA_AM_COMPILE_PYECASOUND
CLEANFILES=pyecasound.so
endif

if ECA_AM_HAVE_PYTHON
install-exec-hook: $(pyecasound_install_list1) $(pyecasound_install_list2)
	$(INSTALL) -d $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
	$(INSTALL) $(pyecasound_install_list1) $(pyecasound_install_list2) $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
else
install-exec-hook:
endif

if ECA_AM_HAVE_PYTHON
uninstall-local:
	rm -f $(pyecasound_uninstall_list1) $(pyecasound_uninstall_list2)
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
else
uninstall-local:
endif
