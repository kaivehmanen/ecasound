# $Revision: 1.1 $, $Date: 2000-12-05 22:08:49 $
Summary:       Software package for multitrack audio processing
Summary(pl):   Oprogramowanie do wielo¶cie¿kowego przetwarzania d¼wiêku
Name:          ecasound
Version:       1.8.5d15
Release:       1
Copyright:     GPL
Group:         Applications/Sound
Group(pl):     Aplikacje/D¼wiêk
Source:        http://ecasound.seul.org/download/%{name}-%{version}.tar.gz
Patch0:        ecasound_sys_readline.patch
Patch1:        ecasound-DESTDIR.patch
BuildRequires: gcc >= 2.95.2
BuildRequires: audiofile-devel >= 0.1.7
BuildRequires: readline-devel
BuildRequires: ncurses-devel
Requires:      lame
Requires:      mpg123
BuildRoot:     %{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Ecasound is the software package for multitrack audio processing.

1. Supported audio inputs/outputs: 
- ALSA - Advanced Linux Sound Architecture (PCM input, output and
  loopback support)
- OSS (/dev/dsp*) - Open Sound System
- RIFF WAVE (.wav) - 8/16bit non-compressed 
- Ecasound Wave Files (.ewf) - simple wrapper format for recording
  purposes
- Raw/headerless sample data (.raw) 
- CDDA (.cdr) - format used on audio-CDs 
- MPEG 1.0/2.0 (layers 1, 2 and 3) (.mp3) - using mpg123 for input and
  lame for output
- Module formats supported by MikMod - XM, IT, S3M, MOD, MTM, 669,
  STM, ULT, FAR, MED, AMF, DSM, IMF, GDM, STX 
- AIFF (.aiff) and Sun/NeXT audio (.au/.snd) formats using libaudiofile: 
- standard input/output streams (stdin, stdout) and named pipes 
2. Effects:
- Various amplifiers, panning, DC-fix, volume normalization 
- Channel mixing and routing 
- Noise gate, two compressors 
- Filters: lowpass, highpass, bandpass, bandreject, resonant lowpass,
  resonant bandpass, resonator, inverse comb 
- Time-based: multitap delay, reverb, fake-stereo 
3. Controllers (for effect parameters):
- sine oscillator 
- generic oscillator (either using an envelope table with static points
  or with linear interpolation) 
- linear envelopes 
- MIDI continuous controllers (CC) 

This package contains ecasound with interactive textmode user program
and utilites which support batch processing:
- ecatools_fixdc: fix DC-offset
- ecatools_normalize: normalize volume level
- ecatools_play: play files using the default output

%description -l pl
N/A

%package -n libecasound
Summary:       Ecasound libraries
Summary(pl):   Biblioteki programu ecasound
Group:         Development/Libraries
Group(pl):     Programowanie/Biblioteki

%description -n libecasound
N/A

%description -l pl -n libecasound
N/A

%package -n libecasound-devel
Summary:       Ecasound headers
Summary(pl):   Pliki nag³ówkowe bibliotek programu ecasound
Group:         Development/Libraries
Group(pl):     Programowanie/Biblioteki

%description -n libecasound-devel
%description -l pl -n libecasound-devel

%prep
%setup -q
%patch0 -p1
%patch1 -p1

%build
automake
autoconf
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" \
	%configure --prefix=%{_prefix} \
    --enable-sys-readline
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
%{__make} DESTDIR=$RPM_BUILD_ROOT mandir=%{_mandir} install-strip

gzip -9nf $RPM_BUILD_ROOT%{_mandir}/*/*

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644, root, root, 755)
%attr(755, root, root) %{_bindir}/ecafixdc
%attr(755, root, root) %{_bindir}/ecanormalize
%attr(755, root, root) %{_bindir}/ecaplay
%attr(755, root, root) %{_bindir}/ecasound
%attr(755, root, root) %{_bindir}/ecasound-config
%{_mandir}/man1/eca*
%{_mandir}/man5/eca*

%files -n libecasound
%defattr(644, root, root, 755)
%dir %{_datadir}/ecasound
%attr(644, root, root) %{_datadir}/ecasound/*
%attr(755, root, root) %{_libdir}/ecasound/libkvutils*.so*
%attr(755, root, root) %{_libdir}/ecasound/libecasound*.so*

%files -n libecasound-devel
%attr(644, root, root) %{_includedir}/ecasound/[^qe]*
%attr(644, root, root) %{_includedir}/kvutils/*
%attr(755, root, root) %{_libdir}/ecasound/libkvutils.a
%attr(755, root, root) %{_libdir}/ecasound/libkvutils.la

%define date	%(echo `LC_ALL="C" date +"%a %b %d %Y"`)
%changelog
* %{date} PLD Team <pld-list@pld.org.pl>
All below listed persons can be reached on <cvs_login>@pld.org.pl

$Log: ecasound.pld.spec,v $
Revision 1.1  2000-12-05 22:08:49  kaiv
New files.

Revision 1.4  2000/11/25 17:53:09  kaiv
*** empty log message ***

* Sat Nov 25 2000 Kai Vehmanen <kaiv@wakkanet.fi>
- ecasignalview added to the package.

Revision 1.3  2000/09/16 13:24:39  kaiv
Final fixes for r14 release.

Revision 1.2  2000/08/29 20:48:57  kaiv
A few new files.

Revision 1.1  2000/06/19 19:13:48  kaiv
-

