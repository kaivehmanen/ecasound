# $Revision: 1.2 $, $Date: 2001-01-17 02:05:50 $

%define python_sitepkgsdir %(echo `python -c "import sys; print (sys.prefix + '/lib/python' + sys.version[:3] + '/site-packages/')"`)
%define python_compile_opt python -O -c "import compileall; compileall.compile_dir('.')"
%define python_compile python -c "import compileall; compileall.compile_dir('.')"

Summary:        Software package for multitrack audio processing
Summary(pl):    Oprogramowanie do wielo¶cie¿kowego przetwarzania d¼wiêku
Name:           ecasound
Version:        1.8.5d15
Release:        1
License:        GPL
Group:          Applications/Sound
Group(de):      Applikationen/Laut
Group(pl):      Aplikacje/D¼wiêk
Source0:        http://ecasound.seul.org/download/%{name}-%{version}.tar.gz
Patch0:         %{name}-lib.patch
BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  ncurses-devel >= 5.0
BuildRequires:  readline-devel
BuildRequires:  alsa-lib-devel
BuildRequires:  audiofile-devel >= 0.1.7
BuildRequires:  python-devel
Requires:       lame
Requires:       mpg123
BuildRoot:      %{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Ecasound is a software package designed for multitrack audio
processing. It can be used for simple tasks like audio playback,
recording and format conversions, as well as for multitrack effect
processing, mixing, recording and signal recycling. Ecasound supports
a wide range of audio inputs, outputs and effect algorithms. Several
open-source audio packages, like for instance ALSA, OSS, mpg123, lame,
libaudiofile and MikMod, are directly supported. One of the advantages
of ecasound's chain-based design is that effects can easily be
combined both in series and in parallel. Oscillators and MIDI-CCs can
be used for controlling effect parameters. Included user-interfaces
are ecasound - a versatile console mode interface, qtecasound - a
Qt-based X-interface and various command-line utils suitable for batch
processing.

%description -l pl
Ecasound jest programem do wielo¶cie¿kowej edycji d¼wiêku, który mo¿e
byæ u¿ywany tak do prostych zadañ typu odtwarzanie i nagrywanie muzyki
czy te¿ konwersji pomiêdzy formatami plików muzycznych jak i do
wielo¶cie¿kowego nak³adania efektów, miksowania (przenikania,
wyciszania), nagrywania i odzyskiwania (w domy¶le odszumiania lub
wyrzucania zniekszta³ceñ) sygna³u.

Ecasound wspiera szerok± gamê ¼róde³ i wyj¶æ d¼wiêku oraz algorytmów
do jego przetwarzania. Ecasound wspiera wiele wolnych (open source)
projektów, takich jak ALSA, OSS, mpg123, lame, libaudiofile czy te¿
MikMod. Jedn± z zalet programu ecasound jest mo¿liwo¶æ ³añcuchowego
(szeregowego) lub równoleg³ego ³±czenia efektów, które mog± byæ
kontrolowane poprzez oscylatory lub MIDI-CC. Pakiet ten zawiera
tekstowy interfejs u¿ytkownika oraz kilka innych narzêdzi nadaj±cych
siê do przetwarzania wsadowego. Dostêpny jest tak¿e graficzny
interfejs u¿ytkownika - qtecasound.

%package -n libecasound
Summary:        Ecasound libraries
Summary(pl):    Biblioteki programu ecasound
Group:          Development/Libraries
Group(de):      Entwicklung/Libraries
Group(fr):      Development/Librairies
Group(pl):      Programowanie/Biblioteki

%description -n libecasound
Ecasound libraries.

%description -l pl -n libecasound
Biblioteki programu ecasound.

%package -n libecasound-devel
Summary:        Ecasound headers
Summary(pl):    Pliki nag³ówkowe bibliotek programu ecasound
Group:          Development/Libraries
Group(de):      Entwicklung/Libraries
Group(fr):      Development/Librairies
Group(pl):      Programowanie/Biblioteki
Requires:       libecasound = %{version}

%description -n libecasound-devel
Ecasound headers.

%description -l pl -n libecasound-devel
Pliki nag³ówkowe bibliotek programu ecasound.

%package -n libecasound-static
Summary:        Ecasound static libraries
Summary(pl):    Biblioteki statyczne programu ecasound
Group:          Development/Libraries
Group(de):      Entwicklung/Libraries
Group(fr):      Development/Librairies
Group(pl):      Programowanie/Biblioteki
Requires:       libecasound-devel = %{version}

%description -n libecasound-static
Ecasound static libraries.

%description -l pl -n libecasound-static
Biblioteki statyczne programu ecasound.

%package plugins
Summary:        Ecasound plugins (ALSA, Audio File Library, aRts)
Summary(pl):    Wtyczki dla programu ecasound (ALSA, Audio File Library, aRts)
Group:          Applications/Sound
Group(de):      Applikationen/Laut
Group(pl):      Aplikacje/D¼wiêk
Requires:       ecasound = %{version}

%description plugins
This package contains ecasound plugins, which give support for ALSA,
Audio File Library and aRts.

%description -l pl plugins
Pakiet ten zawiera wtyczki dla programu ecasound, które umo¿liwiaj±
wspó³pracê z bibliotekami takich projektów jak ALSA, Audio File
Library oraz aRts.

%package -n python-%{name}
Summary:        Python module for Ecasound
Summary(pl):    Modu³ jêzyka Python dla biblioteki programu ecasound
Group:          Development/Languages/Python
Group(de):      Entwicklung/Sprachen/Python
Group(pl):      Programowanie/Jêzyki/Python

%description -n python-%{name}
Python module for Ecasound library.

%description -l pl -n python-%{name}
Modu³ jêzyka Python dla biblioteki programu ecasound.

%prep
%setup -q
%patch -p1

%build
CXXFLAGS="$RPM_OPT_FLAGS -fno-rtti"
automake
%configure \
        --enable-sys-readline
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
%{__make} DESTDIR=$RPM_BUILD_ROOT mandir=%{_mandir} install

( cd pyecasound
  %python_compile_opt
  %python_compile
  install *.pyc *.pyo $RPM_BUILD_ROOT%{python_sitepkgsdir}
)


%clean
rm -rf $RPM_BUILD_ROOT

%post   -n libecasound -p /sbin/ldconfig
%postun -n libecasound -p /sbin/ldconfig

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/ecaconvert
%attr(755,root,root) %{_bindir}/ecafixdc
%attr(755,root,root) %{_bindir}/ecanormalize
%attr(755,root,root) %{_bindir}/ecaplay
%attr(755,root,root) %{_bindir}/ecasignalview
%attr(755,root,root) %{_bindir}/ecasound
%{_mandir}/man1/eca*
%{_mandir}/man5/eca*

%files -n libecasound
%defattr(644,root,root,755)
%dir %{_datadir}/ecasound
%{_datadir}/ecasound/*
%attr(755,root,root) %{_libdir}/libkvutils*.so.*.*
%attr(755,root,root) %{_libdir}/libecasound*.so.*.*

%files -n libecasound-devel
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/ecasound-config
%attr(755,root,root) %{_bindir}/ecasoundc-config
%{_includedir}/ecasound/*
%{_includedir}/kvutils/*
%attr(755,root,root) %{_libdir}/libkvutils.so
%attr(755,root,root) %{_libdir}/libkvutils.la
%attr(755,root,root) %{_libdir}/libecasound*.so
%attr(755,root,root) %{_libdir}/libecasound*.la

%files -n libecasound-static
%defattr(644,root,root,755)
%{_libdir}/libkvutils.a
%{_libdir}/libecasound.a

%files plugins
%defattr(644,root,root,755)
%dir %{_libdir}/ecasound-plugins
%attr(755,root,root) %{_libdir}/ecasound-plugins/lib*.so*
%{_libdir}/ecasound-plugins/lib*.la
%{_libdir}/ecasound-plugins/lib*.a

%files -n python-%{name}
%defattr(644,root,root,755)
%attr(755,root,root) %{python_sitepkgsdir}/*.so
%{python_sitepkgsdir}/*.pyo
%{python_sitepkgsdir}/*.pyc

%define date    %(echo `LC_ALL="C" date +"%a %b %d %Y"`)
%changelog
* %{date} PLD Team <pld-list@pld.org.pl>
All below listed persons can be reached on <cvs_login>@pld.org.pl

$Log: ecasound.pld.spec,v $
Revision 1.2  2001-01-17 02:05:50  kaiv
Latest spec from PLD CVS-tree.

Revision 1.19  2000/12/27 22:29:01  kloczek
- adapterized.

Revision 1.18  2000/12/27 21:10:45  wrobell
- up to 1.8.5d15
- udpated file list
- python subpackage

Revision 1.17  2000/11/12 18:20:14  wrobell
- up to 1.8.4d15

Revision 1.16  2000/10/01 15:48:50  wrobell
- updated file list of ecasound package
- added plugins subpakcage
- updated buildrequires
- added patch for linking plugins without version info

Revision 1.15  2000/09/19 17:21:35  kloczek
- adapterized.

Revision 1.14  2000/09/19 16:25:31  wrobell
- up to 1.8.2r14
- qt stuff is distributed separetely now

Revision 1.13  2000/07/28 10:37:42  kloczek
- release 4.

Revision 1.12  2000/07/28 09:13:03  mis
- typo

Revision 1.11  2000/07/26 09:41:49  wrobell
- finished Polish translation for ecasound package

Revision 1.10  2000/07/13 23:07:14  baggins
- release 3
- rebuild with ncurses 5.1

Revision 1.9  2000/07/13 20:38:52  baggins
- release 2

Revision 1.8  2000/07/03 19:38:34  kloczek
- fixed missing $RPM_BUILD_ROOT on stripping libraries.

Revision 1.7  2000/07/03 18:32:13  kloczek
- added static subpackages,
- fixed resources separiation between devel and main libraries subpackages,
- added stripping shared libraries,
- run /sbin/ldconfig in %pos/%postun for packages with shared libraries,
- added missing Requires rules between main libraris packages and devel
  subpackages,

Revision 1.6  2000/07/03 17:10:29  kloczek
- adapterized.

Revision 1.5  2000/07/03 16:20:15  wrobell
- partial polish description translation

Revision 1.4  2000/07/03 13:50:37  wrobell
- up to 1.8.0d14
- changed ecasound description

Revision 1.3  2000/06/22 09:18:03  wrobell
- typo

Revision 1.2  2000/06/21 19:25:17  kloczek
- spec adapterized,
- rewrited %build,
- NFY (not finished yet).

Revision 1.1  2000/06/19 17:35:29  wrobell
- raw version