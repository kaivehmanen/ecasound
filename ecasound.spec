Buildroot: /tmp/ecasound-build
Prefix: /usr/local
Packager: Kai Vehmanen <kaiv@wakkanet.fi>
Distribution: Red Hat Contrib
Name: ecasound
Version: 1.7.8r12
Release: 1
Copyright: GPL
Source: http://ecasound.seul.org/download/ecasound-1.7.8r12.tar.gz

Summary: ecasound - multitrack audio processing tool
Group: Applications/Sound

%description
Ecasound is a software package designed for multitrack audio
processing. It can be used for simple tasks like audio playback, 
recording and format conversions, as well as for multitrack effect 
processing, mixing, recording and signal recycling. Ecasound supports 
a wide range of audio inputs, outputs and effect algorithms. Several
open-source audio packages, like for instance ALSA, OSS, mpg123, lame, 
libaudiofile and MikMod, are directly supported. One of the advantages 
of ecasound's chain-based design is that effects can easily be 
combined both in series and in parallel. Oscillators and MIDI-CCs 
can be used for controlling effect parameters. Included user-interfaces 
are ecasound - a versatile console mode interface, qtecasound - 
a Qt-based X-interface and various command-line utils suitable for 
batch processing.

%package devel
Summary: Ecasound - Library header files
Group: Applications/Sound
Requires: ecasound

%description devel
Headers files needed for compiling other programs against
ecasound libraries. This is package is not required for installing 
other ecasound RPMs.

%package qt
Summary: Qt-based user-interface to ecasound
Group: X11/Applications/Sound
Requires: ecasound

%description qt
Qt-based X-interface to ecasound.

%prep
%setup -n ecasound-1.7.8r12
%build
./configure --prefix=$RPM_BUILD_ROOT%prefix --disable-static $extra_params
make

%install
make install
make delete-static-libs
make strip-shared-libs

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)
%doc NEWS README INSTALL BUGS Documentation
%doc /usr/local/man/man1/ecasound.1
%doc /usr/local/man/man1/ecatools.1
%doc /usr/local/man/man1/qtecasound.1
%doc /usr/local/man/man1/ecasound-iam.1
%doc /usr/local/man/man5/ecasoundrc.5
/usr/local/bin/ecasound
/usr/local/bin/ecafixdc
/usr/local/bin/ecanormalize
/usr/local/bin/ecaplay
/usr/local/lib/libecasound.*
/usr/local/lib/libkvutils.*
%config /usr/local/share/ecasound

%files devel
%defattr(-, root, root)
/usr/local/include/ecasound
/usr/local/include/kvutils

%files qt
%defattr(-, root, root)
/usr/local/bin/qtecasound
/usr/local/lib/libqtecasound.*

%changelog
* Mon Jun 05 2000 Kai Vehmanen <kaiv@wakkanet.fi>
- Renamed ecatools programs.

* Mon Apr 15 2000 Kai Vehmanen <kaiv@wakkanet.fi>
- Removed dynamic linking to ALSA libraries. You 
  can get ALSA support by recompiling the source-RPM
  package.

* Mon Feb 10 2000 Kai Vehmanen <kaiv@wakkanet.fi>
- Added libqtecasound to ecasound-qt.

* Mon Nov 09 1999 Kai Vehmanen <kaiv@wakkanet.fi>
- A complete reorganization. Ecasound distribution is now 
  divided to three RPMs: ecasound, ecasound-qt and ecasound-devel.

* Mon Nov 08 1999 Kai Vehmanen <kaiv@wakkanet.fi>
- As Redhat stopped the RHCN project, so these rpms 
  are again distributed via Redhat's contrib service
- You can also get these from http://ecasound.seul.org/download

* Sun Aug 15 1999 Kai Vehmanen <kaiv@wakkanet.fi>
- Initial rhcn release.
