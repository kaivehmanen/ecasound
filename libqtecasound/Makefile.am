if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I$(srcdir) -I$(top_srcdir)/kvutils \
		-I$(top_srcdir)/libecasound \
		-I$(top_srcdir)/libqtecasound \
		-I$(qt_includes) -I$(all_includes)

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
libqtecasound_general_include =	qebuttonrow.h \
				qestringdialog.h
libqtecasound_general_moc =	qestringdialog_moc.cpp \
				qebuttonrow_moc.cpp			 

libqtecasound_lobjects_src = 	qesignallevel.cpp \
				qeslider.cpp
libqtecasound_lobjects_include=	qelibraryobject.h \
				qechainoperator.h \
				qecontroller.h \
				qesignallevel.h \
				qeslider.h
libqtecasound_lobjects_moc =	qechainoperator_moc.cpp \
				qecontroller_moc.cpp \
				qesignallevel_moc.cpp \
				qeslider_moc.cpp

libqtecasound_inputs_src = 	qefilenameinput.cpp \
				qeokcancelinput.cpp \
				qeaudioformatinput.cpp \
				qechainoperatorinput.cpp \
				qeobjectmap.cpp \
				qeoperatorconfiguration.cpp \
				qeaudioobjectconfiguration.cpp
libqtecasound_inputs_include = 	qeinput.h \
				qefilenameinput.h \
				qeokcancelinput.h \
				qeaudioformatinput.h \
				qechainoperatorinput.h \
				qeobjectmap.h \
				qeoperatorconfiguration.h \
				qeaudioobjectconfiguration.h
libqtecasound_inputs_moc = 	qeinput_moc.cpp \
				qefilenameinput_moc.cpp \
				qeokcancelinput_moc.cpp \
				qeaudioformatinput_moc.cpp \
				qechainoperatorinput_moc.cpp \
				qeobjectmap_moc.cpp \
				qeoperatorconfiguration_moc.cpp \
				qeaudioobjectconfiguration_moc.cpp
libqtecasound_dialogs_src = 	
libqtecasound_dialogs_include = qedialog.h
libqtecasound_dialogs_moc = 	qedialog_moc.cpp

libqtecasound_common_src = 	$(libqtecasound_general_src) \
				$(libqtecasound_general_moc) \
				$(libqtecasound_lobjects_src) \
				$(libqtecasound_lobjects_moc) \
				$(libqtecasound_inputs_src) \
				$(libqtecasound_inputs_moc) \
				$(libqtecasound_dialogs_src) \
				$(libqtecasound_dialogs_moc) 

libqtecasound_common_include = 	$(libqtecasound_general_include) \
				$(libqtecasound_lobjects_include) \
				$(libqtecasound_inputs_include) \
				$(libqtecasound_dialogs_include)

libqtecasound_la_SOURCES = $(libqtecasound_common_src)
libqtecasound_debug_la_SOURCES = $(libqtecasound_common_src)
libqtecasound_la_LIBADD = -L$(top_builddir)/kvutils -lkvutils
libqtecasound_debug_la_LIBADD = $(libqtecasound_la_LIBADD)
libqtecasound_la_LDFLAGS = -export-dynamic $(libqtecasound_ldflags)
libqtecasound_debug_la_LDFLAGS = $(libqtecasound_la_LDFLAGS)

noinst_HEADERS = $(libqtecasound_common_include)

install-data-local:
	$(INSTALL) -d $(pkgincludedir)
	cd $(srcdir) ; cp $(libqtecasound_common_include) $(pkgincludedir)

# ------
# qt-moc
# ------
dist-hook:
	rm -f $(distdir)/*_moc.cpp


# ---
# general:
qebuttonrow_moc.cpp: $(srcdir)/qebuttonrow.h
	$(MOC) -o qebuttonrow_moc.cpp $(srcdir)/qebuttonrow.h

# ---
# library objects:
qechainoperator_moc.cpp: $(srcdir)/qechainoperator.h
	$(MOC) -o qechainoperator_moc.cpp $(srcdir)/qechainoperator.h
qecontroller_moc.cpp: $(srcdir)/qecontroller.h
	$(MOC) -o qecontroller_moc.cpp $(srcdir)/qecontroller.h
qesignallevel_moc.cpp: $(srcdir)/qesignallevel.h
	$(MOC) -o qesignallevel_moc.cpp $(srcdir)/qesignallevel.h
qeslider_moc.cpp: $(srcdir)/qeslider.h
	$(MOC) -o qeslider_moc.cpp $(srcdir)/qeslider.h

# ---
# inputs:
qeaudioformatinput_moc.cpp: $(srcdir)/qeaudioformatinput.h
	$(MOC) -o qeaudioformatinput_moc.cpp $(srcdir)/qeaudioformatinput.h
qechainoperatorinput_moc.cpp: $(srcdir)/qechainoperatorinput.h
	$(MOC) -o qechainoperatorinput_moc.cpp $(srcdir)/qechainoperatorinput.h
qeoperatorconfiguration_moc.cpp: $(srcdir)/qeoperatorconfiguration.h
	$(MOC) -o qeoperatorconfiguration_moc.cpp $(srcdir)/qeoperatorconfiguration.h
qeaudioobjectconfiguration_moc.cpp: $(srcdir)/qeaudioobjectconfiguration.h
	$(MOC) -o qeaudioobjectconfiguration_moc.cpp $(srcdir)/qeaudioobjectconfiguration.h
qefilenameinput_moc.cpp: $(srcdir)/qefilenameinput.h
	$(MOC) -o qefilenameinput_moc.cpp $(srcdir)/qefilenameinput.h
qeinput_moc.cpp: $(srcdir)/qeinput.h
	$(MOC) -o qeinput_moc.cpp $(srcdir)/qeinput.h
qeobjectmap_moc.cpp: $(srcdir)/qeobjectmap.h
	$(MOC) -o qeobjectmap_moc.cpp $(srcdir)/qeobjectmap.h
qeokcancelinput_moc.cpp: $(srcdir)/qeokcancelinput.h
	$(MOC) -o qeokcancelinput_moc.cpp $(srcdir)/qeokcancelinput.h

# ---
# dialogs:
qedialog_moc.cpp: $(srcdir)/qedialog.h
	$(MOC) -o qedialog_moc.cpp $(srcdir)/qedialog.h
qestringdialog_moc.cpp: $(srcdir)/qestringdialog.h
	$(MOC) -o qestringdialog_moc.cpp $(srcdir)/qestringdialog.h
