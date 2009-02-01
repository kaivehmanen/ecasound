# ----------------------------------------------------------------------
# File: ecasound/Documentation/Makefile.am
# Description: Ecasound documentation files
# License: GPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

prefix = @prefix@
mandir = @mandir@
PACKAGE = @PACKAGE@
VERSION = @VERSION@
SUBDIRS = users_guide programmers_guide

# ---------------------------------------------------------------------
# Files going into distribution:
EXTRA_DIST = 	$(srcdir)/ecasound.1 \
		$(srcdir)/ecasound_manpage.html \
	    	$(srcdir)/ecasound_manpage.yo \
		$(srcdir)/ecasoundrc.5 \
		$(srcdir)/ecasoundrc_manpage.html \
	    	$(srcdir)/ecasoundrc_manpage.yo \
	    	$(srcdir)/ecasound-iam.1 \
	    	$(srcdir)/ecasound-iam_manpage.html \
	    	$(srcdir)/ecasound-iam_manpage.yo \
	    	$(srcdir)/ecatools.1 \
	    	$(srcdir)/ecatools_manpage.html \
		$(srcdir)/ecatools_manpage.yo \
	    	$(srcdir)/ecaconvert.1 \
	    	$(srcdir)/ecafixdc.1 \
	    	$(srcdir)/ecalength.1 \
	    	$(srcdir)/ecalength_manpage.html \
	    	$(srcdir)/ecalength_manpage.yo \
	    	$(srcdir)/ecaplay.1 \
	    	$(srcdir)/ecamonitor.1 \
	    	$(srcdir)/ecanormalize.1 \
	    	$(srcdir)/ecasignalview.1 \
		$(srcdir)/index.html \
	    	$(srcdir)/examples.html \
		$(srcdir)/manpages.html \
		$(srcdir)/tutorials.html \
		$(srcdir)/style_ecasound.css \
		$(srcdir)/edi-list.txt

# ---------------------------------------------------------------------
# local targets for generating docs from latex and yodl sources

all:

docs: docs_guides docs_files

docs_guides:
	cd programmers_guide; $(MAKE) $(AM_MAKEFLAGS) docs
	cd users_guide; $(MAKE) $(AM_MAKEFLAGS) docs

docs_files: 	ecasound.1 ecasound_manpage.html \
	    	ecasoundrc_manpage.html ecasoundrc.5 \
		ecatools.1 ecatools_manpage.man ecatools_manpage.html \
		ecaconvert.1 \
		ecafixdc.1 \
		ecalength.1 ecalength_manpage.man ecalength_manpage.html \
		ecamonitor.1 \
		ecanormalize.1 \
		ecaplay.1 \
		ecasignalview.1 \
		ecasound-iam_manpage.html \
		ecasound-iam.1

# ---------------------------------------------------------------------
# Man pages

ecasound.1: ecasound_manpage.man
	cp ecasound_manpage.man ecasound.1
ecatools.1: ecatools_manpage.man
	cp ecatools_manpage.man ecatools.1
ecaconvert.1: ecatools_manpage.man
	cp ecatools_manpage.man ecaconvert.1
ecafixdc.1: ecatools_manpage.man
	cp ecatools_manpage.man ecafixdc.1
ecalength.1: ecalength_manpage.man
	cp ecalength_manpage.man ecalength.1
ecamonitor.1: ecatools_manpage.man
	cp ecatools_manpage.man ecamonitor.1
ecanormalize.1: ecatools_manpage.man
	cp ecatools_manpage.man ecanormalize.1
ecaplay.1: ecatools_manpage.man
	cp ecatools_manpage.man ecaplay.1
ecasignalview.1: ecatools_manpage.man
	cp ecatools_manpage.man ecasignalview.1
ecasound-iam.1: ecasound-iam_manpage.man
	cp ecasound-iam_manpage.man ecasound-iam.1
ecasoundrc.5: ecasoundrc_manpage.man
	cp ecasoundrc_manpage.man ecasoundrc.5

%.man: $(srcdir)/%.yo
	yodl2man -o $@ $<

%.html: $(srcdir)/%.yo
	yodl2html -o $@ $<

# ---------------------------------------------------------------------
# doxygen (target removed 2009/Feb)
#doxygen:
#	doxygen doxygen_libkvutils_config
#	doxygen doxygen_libecasound_config

# ---------------------------------------------------------------------
# Clean targets
clean:
clean-docs:
	rm -f 	ecasound.1 \
	      	ecasound_manpage.html \
		ecatools.1 \
		ecaconvert.1 \
		ecafixdc.1 \
		ecalength.1 \
		ecamonitor.1 \
		ecanormalize.1 \
		ecaplay.1 \
		ecasignalview.1 \
	      	ecatools_manpage.html \
	      	ecasoundrc.5 \
	      	ecasoundrc_manpage.html \
	      	ecasound-iam.1 \
		ecasound-iam_manpage.html

# ---------------------------------------------------------------------
# Install targets
install-data-local:
	mkdir -p $(DESTDIR)$(mandir)/man1
	mkdir -p $(DESTDIR)$(mandir)/man5
	cp $(srcdir)/ecasound.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecatools.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecaconvert.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecafixdc.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecalength.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecamonitor.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecanormalize.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecaplay.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecasignalview.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecasound-iam.1 $(DESTDIR)$(mandir)/man1
	cp $(srcdir)/ecasoundrc.5 $(DESTDIR)$(mandir)/man5

# ---------------------------------------------------------------------
# Uninstall targets
uninstall-local:
	rm -f $(DESTDIR)$(mandir)/man1/ecasound.1
	rm -f $(DESTDIR)$(mandir)/man1/ecatools.1
	rm -f $(DESTDIR)$(mandir)/man1/ecaconvert.1
	rm -f $(DESTDIR)$(mandir)/man1/ecafixdc.1
	rm -f $(DESTDIR)$(mandir)/man1/ecalength.1
	rm -f $(DESTDIR)$(mandir)/man1/ecamonitor.1
	rm -f $(DESTDIR)$(mandir)/man1/ecanormalize.1
	rm -f $(DESTDIR)$(mandir)/man1/ecaplay.1
	rm -f $(DESTDIR)$(mandir)/man1/ecasignalview.1
	rm -f $(DESTDIR)$(mandir)/man1/ecasound-iam.1
	rm -f $(DESTDIR)$(mandir)/man5/ecasoundrc.5
