if INCLUDE_DEBUG
INSTALL_STRIP_FLAG =
else
INSTALL_STRIP_FLAG = -s
endif

INCLUDES = -I. -I$(qt_includes) -I$(all_includes)

if INCLUDE_DEBUG
bin_PROGRAMS = ecawave_debug
else
bin_PROGRAMS = ecawave
endif

ecawave_src_files = 	ecawave.cpp \
			version.cpp \
			resources.cpp \
		 	qesession.cpp \
			qefile.cpp \
			qeacceptinput.cpp \
			qeopenfiledialog.cpp \
			qesavefiledialog.cpp \
			qewaveform.cpp \
			qestatusbar.cpp \
			qeevent.cpp \
			qenonblockingevent.cpp \
			qeplayevent.cpp \
			qesaveevent.cpp \
			qechainopevent.cpp \
			qecutevent.cpp \
			qecopyevent.cpp \
			qepasteevent.cpp
ecawave_src_mocs = 	qesession_moc.cpp \
			qefile_moc.cpp \
			qeacceptinput_moc.cpp \
			qeopenfiledialog_moc.cpp \
			qesavefiledialog_moc.cpp \
			qewaveform_moc.cpp \
			qestatusbar_moc.cpp \
			qechainopevent_moc.cpp
ecawave_include_files = ecawave.h \
			version.h \
			resources.h \
			qesession.h \
			qefile.h \
			qeacceptinput.h \
			qeopenfiledialog.h \
			qesavefiledialog.h \
			qewaveform.h \
			qestatusbar.h \
			qeevent.h \
			qeblockingevent.h \
			qenonblockingevent.h \
			qeplayevent.h \
			qesaveevent.h \
			qechainopevent.h \
			qecutevent.h \
			qecopyevent.h \
			qepasteevent.h

ecawave_SOURCES = $(ecawave_src_files) $(ecawave_include_files) $(ecawave_src_mocs)
ecawave_debug_SOURCES = $(ecawave_SOURCES)

ecawave_LDADD = -lecasound -lqtecasound -L$(qt_libraries) -lqt -lkvutils
ecawave_LDFLAGS = $(all_libraries)
ecawave_debug_LDADD = -lecasound_debug -lqtecasound_debug -L$(qt_libraries) -lqt -lkvutils
ecawave_debug_LDFLAGS = $(ecawave_LDFLAGS)

# ------
# qt-moc
# ------
dist-hook:
	rm -f $(distdir)/*_moc.cpp

qesession_moc.cpp: $(srcdir)/qesession.h
	$(MOC) -o qesession_moc.cpp $(srcdir)/qesession.h

qefile_moc.cpp: $(srcdir)/qefile.h
	$(MOC) -o qefile_moc.cpp $(srcdir)/qefile.h

qeacceptinput_moc.cpp: $(srcdir)/qeacceptinput.h
	$(MOC) -o qeacceptinput_moc.cpp $(srcdir)/qeacceptinput.h

qeopenfiledialog_moc.cpp: $(srcdir)/qeopenfiledialog.h
	$(MOC) -o qeopenfiledialog_moc.cpp $(srcdir)/qeopenfiledialog.h

qesavefiledialog_moc.cpp: $(srcdir)/qesavefiledialog.h
	$(MOC) -o qesavefiledialog_moc.cpp $(srcdir)/qesavefiledialog.h

qechainopevent_moc.cpp: $(srcdir)/qechainopevent.h
	$(MOC) -o qechainopevent_moc.cpp $(srcdir)/qechainopevent.h

qewaveform_moc.cpp: $(srcdir)/qewaveform.h
	$(MOC) -o qewaveform_moc.cpp $(srcdir)/qewaveform.h


qestatusbar_moc.cpp: $(srcdir)/qestatusbar.h
	$(MOC) -o qestatusbar_moc.cpp $(srcdir)/qestatusbar.h

