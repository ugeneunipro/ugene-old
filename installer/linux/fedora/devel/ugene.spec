Name:    ugene
Summary: Integrated bioinformatics toolkit
Version: 1.5.1
Release: 1%{?dist}
#The entire source code is GPLv2+ except:
#file src/libs_3rdparty/qtbindings_core/src/qtscriptconcurrent.h which is GPLv2
#files in src/plugins_3rdparty/script_debuger/src/qtscriptdebug/ which are GPLv2
License: GPLv2+ and GPLv2
Group:   Applications/Engineering
URL:     http://ugene.unipro.ru
Source0: http://ugene.unipro.ru/downloads/%{name}-%{version}.tar.gz

Buildroot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: qt-devel >= 4.5.0 zlib-devel desktop-file-utils
#We need strict versions of qt for correct work of src/libs_3rdparty/qtbindings_*
Requires:  qt >= 4.5.0 qt-x11 >= 4.5.0

%description
Unipro UGENE is a cross-platform visual environment for DNA and protein
sequence analysis. UGENE integrates the most important bioinformatics
computational algorithms and provides an easy-to-use GUI for performing
complex analysis of the genomic data. One of the main features of UGENE
is a designer for custom bioinformatics workflows.

%prep
%setup -q

%build
%_bindir/qmake-qt4 -r \
        INSTALL_BINDIR=%{_bindir} \
        INSTALL_LIBDIR=%{_libdir} \
        INSTALL_DATADIR=%{_datadir} \
        INSTALL_MANDIR=%{_mandir}

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install INSTALL_ROOT=%{buildroot}
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{name}.desktop

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_libdir}/%{name}/
%{_datadir}/applications/*
%{_datadir}/pixmaps/*
%{_datadir}/%{name}/
%{_mandir}/man1/*
%doc COPYRIGHT LICENSE

%changelog
* Fri Jul 17 2009 Ivan Efremov <iefremov@unipro.ru> - 1.5.1-1 
- Upstream version change
- Fix for lrelease removed due to upstream package changes

* Fri Jul 10 2009 Ivan Efremov <iefremov@unipro.ru> - 1.5.0-3
- desktop-file-utils added to dependencies

* Mon Jul 06 2009 Ivan Efremov <iefremov@unipro.ru> - 1.5.0-1
- Upstream version change
- Needed Qt versions bumped up
- Fix for lrelease updated due to upstream package changes

* Tue Mar 24 2009 Ivan Efremov <iefremov@unipro.ru> - 1.4.1-1
- Upstream version change

* Fri Mar 06 2009 Ivan Efremov <iefremov@unipro.ru> - 1.4.0-1
- Upstream version change

* Mon Feb 02 2009 Ivan Efremov <iefremov@unipro.ru> - 1.3.3-1
- Initial release of rpm
