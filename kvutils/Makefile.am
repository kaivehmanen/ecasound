AUTOMAKE_OPTIONS = foreign

# ---
# Makefile.am for building libkvutils.a
# ---

if INCLUDE_DEBUG
lib_LTLIBRARIES = libkvutils_debug.la
else
lib_LTLIBRARIES = libkvutils.la
endif

kvutil_sources = 	kvutils.cpp \
			message_item.cpp \
			com_line.cpp \
		 	value_queue.cpp \
			kvu_numtostr.cpp \
			kvu_rtcaps.cpp \
			locks.cpp \
			procedure_timer.cpp \
			temporary_file_directory.cpp

kvutil_headers = 	kvutils.h \
			message_item.h \
			com_line.h \
			value_queue.h \
			kvu_numtostr.h \
			kvu_rtcaps.h \
			definition_by_contract.h \
			object_queue.h \
			locks.h \
			procedure_timer.h \
			temporary_file_directory.h \
			dbc.h

libkvutils_la_SOURCES = $(kvutil_sources) $(kvutil_headers)
libkvutils_la_LDFLAGS = -version-info 3:0:1

libkvutils_debug_la_SOURCES = $(libkvutils_la_SOURCES)
libkvutils_debug_la_LDFLAGS = $(libkvutils_la_LDFLAGS)

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
