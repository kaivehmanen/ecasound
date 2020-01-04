# ----------------------------------------------------------------------
# File: ecasound/pyecasound/Makefile.am
# Description: Python implmentation of the Ecasound Control Interface
# License: LGPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

AUTOMAKE_OPTIONS 		= foreign
EXTRA_DIST 			= eci.py pyeca.py ecacontrol.py \
				  test1_stresstest.py \
				  test2_stresstest.py

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if ECA_AM_PYECASOUND_INSTALL
pyecasound_install_list1 = 	$(srcdir)/eci.py \
				$(srcdir)/pyeca.py \
				$(srcdir)/ecacontrol.py
pyecasound_uninstall_list1 =    $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/eci.py \
				$(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/pyeca.py \
				$(DESTDIR)$(ECA_S_PYTHON_DLMODULES)/ecacontrol.py
endif

if ECA_AM_PYECASOUND_INSTALL
TESTS				= test1_stresstest.py \
				  test2_stresstest.py
endif

# ----------------------------------------------------------------------
# header files
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
# source files
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
# hooks
# ----------------------------------------------------------------------

if ECA_AM_PYECASOUND_INSTALL
install-exec-hook: $(pyecasound_install_list1)
	$(INSTALL) -d $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
	$(INSTALL) $(pyecasound_install_list1) $(DESTDIR)$(ECA_S_PYTHON_DLMODULES)
else
install-exec-hook:
endif

if ECA_AM_PYECASOUND_INSTALL
uninstall-local:
	rm -f $(pyecasound_uninstall_list1)
	rmdir $(DESTDIR)$(ECA_S_PYTHON_DLMODULES) || echo "Skipping non-empty directory"
else
uninstall-local:
endif
