Buildroot: /tmp/ecawave-build
Prefix: /usr/local
Packager: Kai Vehmanen <kaiv@wakkanet.fi>
Distribution: Red Hat Contrib
Name: ecawave
Version: METAVERSION
Release: 1
Copyright: GPL
Source: http://ecasound.seul.org/download/ecawave-%{version}.tar.gz
Summary: ecawave - graphical audio file editor
Group: X11/Applications/Sound
Requires: ecasound
Requires: ecasound-qt

%description
Ecawave is a simple graphical audio file editor. The user-interface is
based on Qt libraries, while almost all audio functionality is taken 
directly from ecasound libraries. As ecawave is designed for editing 
large audio files, all processing is done direct-to-disk. Simple 
waveform caching is used to speed-up file operations. Ecawave supports 
all audio file formats and effect algorithms provided by ecasound
libraries. This includes ALSA and OSS soundcard support and common 
file formats like wav, cdr, raw, aiff, mp3, etc.

%prep
%setup -n ecawave-METAVERSION
%build
./configure --prefix=$RPM_BUILD_ROOT%prefix
make

%install
make install

%files
%defattr(-, root, root)
%doc NEWS README INSTALL Documentation
%doc /usr/local/man/man1/ecawave.1
/usr/local/bin/ecawave

%changelog
* Thu Jan 27 2000 Kai Vehmanen <kaiv@wakkanet.fi>
- Initial RPM-release.
