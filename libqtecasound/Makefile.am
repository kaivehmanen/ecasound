if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I. -I$(qt_includes) -I$(all_includes)

if INCLUDE_DEBUG
libqtecasound_ldflags = -version-info 0:0:0 -lkvutils
else
libqtecasound_ldflags = -s -version-info 0:0:0 -lkvutils
endif

if INCLUDE_DEBUG
libqtecasound_target = libqtecasound_debug.la
else
libqtecasound_target = libqtecasound.la
endif

if COMPILE_QTECASOUND
lib_LTLIBRARIES = $(libqtecasound_target)
endif

libqtecasound_general_src = 	qebuttonrow.cpp \
			    	qestringdialog.cpp

libqtecasound_general_moc =	qestringdialog_moc.cpp \
				qebuttonrow_moc.cpp			 

libqtecasound_include =		qebuttonrow.h \
				qestringdialog.h

libqtecasound_common_src = 	$(libqtecasound_general_src) \
				$(libqtecasound_general_moc)

libqtecasound_la_SOURCES = $(libqtecasound_common_src)
libqtecasound_debug_la_SOURCES = $(libqtecasound_common_src)
libqtecasound_la_LIBADD = -L$(top_builddir)/kvutils -lkvutils
libqtecasound_debug_la_LIBADD = $(libqtecasound_la_LIBADD)
libqtecasound_la_LDFLAGS = --export-dynamic $(libqtecasound_ldflags)
libqtecasound_debug_la_LDFLAGS = $(libqtecasound_la_LDFLAGS)

noinst_HEADERS = $(libqtecasound_include)

install-data-local:
	$(INSTALL) -d $(pkgincludedir)
	cd $(srcdir) ; cp $(libqtecasound_include) $(pkgincludedir)

# ------
# qt-moc
# ------
dist-hook:
	rm -f $(distdir)/*_moc.cpp

qebuttonrow_moc.cpp: $(srcdir)/qebuttonrow.h
	$(MOC) -o qebuttonrow_moc.cpp $(srcdir)/qebuttonrow.h
qestringdialog_moc.cpp: $(srcdir)/qestringdialog.h
	$(MOC) -o qestringdialog_moc.cpp $(srcdir)/qestringdialog.h
