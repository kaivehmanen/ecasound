AUTOMAKE_OPTIONS = foreign

# ---
# Makefile.am for building libkvutils.a
# ---

if ECA_AM_DEBUG_MODE
lib_LTLIBRARIES = libkvutils3_debug.la
else
lib_LTLIBRARIES = libkvutils3.la
endif

kvutil_sources = 	kvu_utils.cpp \
			kvu_message_item.cpp \
			kvu_com_line.cpp \
		 	kvu_value_queue.cpp \
			kvu_numtostr.cpp \
			kvu_rtcaps.cpp \
			kvu_locks.cpp \
			kvu_threads.cpp \
			kvu_procedure_timer.cpp \
			kvu_temporary_file_directory.cpp

kvutil_headers = 	kvu_utils.h \
			kvu_message_item.h \
			kvu_com_line.h \
			kvu_value_queue.h \
			kvu_numtostr.h \
			kvu_rtcaps.h \
			kvu_definition_by_contract.h \
			kvu_object_queue.h \
			kvu_locks.h \
			kvu_threads.h \
			kvu_procedure_timer.h \
			kvu_temporary_file_directory.h \
			kvu_dbc.h

libkvutils3_la_SOURCES = $(kvutil_sources) $(kvutil_headers)
libkvutils3_la_LDFLAGS = -version-info @LIBKVUTILS_VERSION@:0:0

libkvutils3_debug_la_SOURCES = $(libkvutils3_la_SOURCES)
libkvutils3_debug_la_LDFLAGS = $(libkvutils3_la_LDFLAGS)

noinst_HEADERS = $(kvutil_headers)

# ---------------------------------------------------------------------
# Install targets
install-data-local:
	$(INSTALL) -d $(DESTDIR)$(includedir)/kvutils@LIBKVUTILS_VERSION@
	cd $(srcdir) ; cp  $(kvutil_headers) $(DESTDIR)$(includedir)/kvutils@LIBKVUTILS_VERSION@

# ---------------------------------------------------------------------
# Uninstall targets
uninstall-local:
	cd $(DESTDIR)$(includedir)/kvutils@LIBKVUTILS_VERSION@ && \
	rm -f   $(kvutil_headers)
	rmdir   $(DESTDIR)$(includedir)/kvutils@LIBKVUTILS_VERSION@
