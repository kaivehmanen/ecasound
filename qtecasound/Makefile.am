if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I. -I$(top_srcdir)/kvutils -I$(top_srcdir)/libecasound/include -I$(qt_includes) $(all_includes)

if COMPILE_QTECASOUND
qt_binaries = qtecasound
endif

if INCLUDE_DEBUG
qt_binaries = qtecasound_debug
endif

bin_PROGRAMS = $(qt_binaries)

# ---
# qtecasound
# ---
if COMPILE_QTECASOUND
ecasound_qt_src = 	eca-qtdebug.cpp \
			eca-qtinte.cpp \
			eca-qtlevelmeter.cpp \
			eca-qtmain.cpp \
			eca-qtrtposition.cpp \
			eca-qtsignallevel.cpp \
			qtdebug_if.cpp \
			eca-qtsession.cpp \
			eca-qtchainsetup.cpp \
			eca-qtchain.cpp \
			eca-qtiodevdialog.cpp \
			eca-qtchainselectdialog.cpp \
			qestringdialog.cpp \
			qebuttonrow.cpp

ecasound_qt_includes = 	eca-qtdebug.h \
			eca-qtinte.h \
			eca-qtlevelmeter.h \
			eca-qtchainselectdialog.h \
			eca-qtiodevdialog.h \
			eca-qtchain.h \
		 	eca-qtchainsetup.h \
			eca-qtsession.h  \
			qtdebug_if.h \
			eca-qtrtposition.h \
			eca-qtmain.h \
			eca-qtsignallevel.h \
			qestringdialog.h \
			qebuttonrow.h

ecasound_qt_mocs = 	eca-qtdebug_moc.cpp \
			eca-qtinte_moc.cpp \
			eca-qtchainselectdialog_moc.cpp \
			eca-qtiodevdialog_moc.cpp \
			eca-qtchain_moc.cpp \
			eca-qtchainsetup_moc.cpp \
			eca-qtsession_moc.cpp \
			eca-qtsignallevel_moc.cpp \
			eca-qtrtposition_moc.cpp \
			qestringdialog_moc.cpp \
			qebuttonrow_moc.cpp

qtecasound_SOURCES = $(ecasound_qt_src) $(ecasound_qt_mocs) $(ecasound_qt_includes)
qtecasound_debug_SOURCES = $(ecasound_qt_src) $(ecasound_qt_mocs) $(ecasound_qt_includes)
#qtecasound_LDFLAGS = -s -lstdc++ 
qtecasound_LDADD =  -lqt -L$(top_builddir)/kvutils \
		    -L$(top_builddir)/kvutils/.libs \
		    -lkvutils $(top_builddir)/libecasound/src/libecasound.la
qtecasound_debug_LDADD =  -L$(top_builddir)/kvutils \
			  -L$(top_builddir)/kvutils/.libs \
			  -lkvutils -L$(qt_libraries) \
			  -lqt $(top_builddir)/libecasound/src/libecasound_debug.la
qtecasound_LDFLAGS = $(all_libraries)
qtecasound_debug_LDFLAGS = $(qtecasound_LDFLAGS)
#qtecasound_LDADD   = -lm -lpthread -lqt $(top_srcdir)/libecasound/src/libecasound.la
#qtecasound_METASOURCES = USE_AUTOMOC

# qt-moc: 

DISTCLEANFILES = $(ecasound_qt_mocs)
qtecasound_METASOURCES = $(ecasound_qt_mocs)
noinst_HEADERS = $(ecasound_qt_includes)

dist-hook:
	rm -f $(distdir)/*_moc.cpp

#qlistview-dumb.cpp: qlistview-dumb.h
#	$(MOC) -o qlistview-dumb_moc.cpp qlistview-dumb.h
qestringdialog_moc.cpp: $(srcdir)/qestringdialog.h
	$(MOC) -o qestringdialog_moc.cpp $(srcdir)/qestringdialog.h
qebuttonrow_moc.cpp: $(srcdir)/qebuttonrow.h
	$(MOC) -o qebuttonrow_moc.cpp $(srcdir)/qebuttonrow.h
eca-qtinte_moc.cpp: $(srcdir)/eca-qtinte.h
	$(MOC) -o eca-qtinte_moc.cpp $(srcdir)/eca-qtinte.h
eca-qtdebug_moc.cpp: $(srcdir)/eca-qtdebug.h
	$(MOC) -o eca-qtdebug_moc.cpp $(srcdir)/eca-qtdebug.h
eca-qtsignallevel_moc.cpp: $(srcdir)/eca-qtsignallevel.h
	$(MOC) -o eca-qtsignallevel_moc.cpp $(srcdir)/eca-qtsignallevel.h
eca-qtrtposition_moc.cpp: $(srcdir)/eca-qtrtposition.h
	$(MOC) -o eca-qtrtposition_moc.cpp $(srcdir)/eca-qtrtposition.h
eca-qtsession_moc.cpp: $(srcdir)/eca-qtsession.h
	$(MOC) -o eca-qtsession_moc.cpp $(srcdir)/eca-qtsession.h
eca-qtchainsetup_moc.cpp: $(srcdir)/eca-qtchainsetup.h
	$(MOC) -o eca-qtchainsetup_moc.cpp $(srcdir)/eca-qtchainsetup.h
eca-qtchain_moc.cpp: $(srcdir)/eca-qtchain.h
	$(MOC) -o eca-qtchain_moc.cpp $(srcdir)/eca-qtchain.h
eca-qtiodevdialog_moc.cpp: $(srcdir)/eca-qtiodevdialog.h
	$(MOC) -o eca-qtiodevdialog_moc.cpp $(srcdir)/eca-qtiodevdialog.h
eca-qtchainselectdialog_moc.cpp: $(srcdir)/eca-qtchainselectdialog.h
	$(MOC) -o eca-qtchainselectdialog_moc.cpp $(srcdir)/eca-qtchainselectdialog.h
endif
# --- qtecasound, end
