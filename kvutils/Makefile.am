AUTOMAKE_OPTIONS = foreign

# ---
# Makefile.am for building libkvutils.a
# ---
CXXFLAGS+=-D_REENTRANT -s -O2 -ffast-math -I$(srcdir)

lib_LTLIBRARIES = libkvutils.la

kvutil_sources = 	kvutils.cpp \
			message_item.cpp \
			com_line.cpp \
		 	value_queue.cpp \
			kvu_numtostr.cpp \
			locks.cpp \
			procedure_timer.cpp \
			temporary_file_directory.cpp

kvutil_headers = 	kvutils.h \
			message_item.h \
			com_line.h \
			value_queue.h \
			kvu_numtostr.h \
			definition_by_contract.h \
			object_queue.h \
			locks.h \
			procedure_timer.h \
			temporary_file_directory.h \
			dbc.h

libkvutils_la_SOURCES = $(kvutil_sources) $(kvutil_headers)
libkvutils_la_LDFLAGS = -version-info 2:0:0

DIST_COMMON = 	README \
		COPYING \
		INSTALL \
		Makefile.in \
		Makefile.am

noinst_HEADERS = $(kvutil_headers)

# ---------------------------------------------------------------------
# Install targets
install-data-local:
	$(INSTALL) -d $(DESTDIR)$(includedir)/kvutils
	cd $(srcdir) ; cp  $(kvutil_headers) $(DESTDIR)$(includedir)/kvutils

# ---------------------------------------------------------------------
# Uninstall targets
uninstall-local:
	cd $(DESTDIR)$(includedir)/kvutils ; \
	rm -f   $(kvutil_headers)
	rmdir   $(DESTDIR)$(includedir)/kvutils
