#
# Makefile for Documentation/ subdir 
#
# ---------------------------------------------------------------------
prefix = @prefix@
mandir = @mandir@
PACKAGE = @PACKAGE@
VERSION = @VERSION@
SUBDIRS = users_guide programmers_guide

#distdir = ../$(PACKAGE)-$(VERSION)
#top_distdir = $(distdir)
# ---------------------------------------------------------------------
# Files going into distribution:
DISTFILES = 	Makefile.am \
		Makefile.in \
		ecasound.1 \
		ecasound_manpage.html \
	    	ecasound_manpage.yo \
		ecasoundrc.5 \
		ecasoundrc_manpage.html \
	    	ecasoundrc_manpage.yo \
	    	ecasound-iam.1 \
	    	ecasound-iam_manpage.html \
	    	ecasound-iam_manpage.yo \
	    	ecatools.1 \
	    	ecatools_manpage.html \
		ecatools_manpage.yo \
	    	ecaconvert.1 \
	    	ecafixdc.1 \
	    	ecalength.1 \
	    	ecalength_manpage.html \
	    	ecalength_manpage.yo \
	    	ecaplay.1 \
	    	ecanormalize.1 \
	    	ecasignalview.1 \
	    	examples.html \
		index.html \
		manpages.html \
		edi-list.txt

# ---------------------------------------------------------------------
# local targets for generating docs from latex and yodl sources

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

%.man: %.yo
	yodl2man $<

%.html: %.yo
	yodl2html $<

# ---------------------------------------------------------------------
# doxygen
doxygen:
	doxygen doxygen_libkvutils_config
	doxygen doxygen_libecasound_config

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
	rm -f $(DESTDIR)$(mandir)/man1/ecanormalize.1
	rm -f $(DESTDIR)$(mandir)/man1/ecaplay.1
	rm -f $(DESTDIR)$(mandir)/man1/ecasignalview.1
	rm -f $(DESTDIR)$(mandir)/man1/ecasound-iam.1
	rm -f $(DESTDIR)$(mandir)/man5/ecasoundrc.5
