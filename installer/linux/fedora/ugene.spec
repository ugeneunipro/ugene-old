Name:    ugene
Summary: Integrated bioinformatics toolkit
Version: 1.11.4
Release: 2%{?dist}
#The entire source code is GPLv2+ except:
#file src/libs_3rdparty/qtbindings_core/src/qtscriptconcurrent.h which is GPLv2
#files in src/plugins_3rdparty/script_debuger/src/qtscriptdebug/ which are GPLv2
License: GPLv2+ and GPLv2
Group:   Applications/Engineering
URL:     http://ugene.unipro.ru
Source0: http://ugene.unipro.ru/downloads/%{name}-%{version}.tar.gz

BuildRequires: qt4-devel pkgconfig(QtWebKit) zlib-devel desktop-file-utils
#We need strict versions of qt for correct work of src/libs_3rdparty/qtbindings_*
%{?_qt4_version:Requires: qt4%{?_isa} >= %{_qt4_version}}
Provides: bundled(sqlite)
Provides: bundled(samtools)

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
        INSTALL_MANDIR=%{_mandir} \
%if 0%{?_with_non_free}
        UGENE_WITHOUT_NON_FREE=0 \
%else
        UGENE_WITHOUT_NON_FREE=1 \
%endif
        UGENE_EXCLUDE_LIST_ENABLED=1
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{name}.desktop

%files
%{_bindir}/*
%{_libdir}/%{name}/
%{_datadir}/applications/*
%{_datadir}/pixmaps/*
%{_datadir}/%{name}/
%{_mandir}/man1/*
%doc COPYRIGHT LICENSE LICENSE.3rd_party 

%changelog
* Tue Jan 22 2013 Yulia Algaer <yalgaer@unipro.ru> 1.11.4-1
- Upstream version change

* Tue Nov 27 2012 Rex Dieter <rdieter@fedoraproject.org> 1.11.3-2
- fix/update qt-related dependencies

* Thu Nov 2 2012 Yulia Algaer <yalgaer@unipro.ru> - 1.11.3-1 
- Upstream version change

* Mon Oct 3 2012 Yulia Algaer <yalgaer@unipro.ru> - 1.11.2-1 
- Upstream version change

* Mon Sep 21 2009 Ivan Efremov <iefremov@unipro.ru> - 1.5.2-1 
- Upstream version change

* Fri Jul 17 2009 Ivan Efremov <iefremov@unipro.ru> - 1.5.1-1 
- Upstream version change
- Fix for lrelease removed due to upstream package changes

* Mon Jul 06 2009 Ivan Efremov <iefremov@unipro.ru> - 1.5.0-1
- Upstream version change
- Needed Qt versions bumped up
- Fix for lrelease updated due to upstream package changes
- desktop-file-utils added to dependencies

* Tue Mar 24 2009 Ivan Efremov <iefremov@unipro.ru> - 1.4.1-1
- Upstream version change

* Fri Mar 06 2009 Ivan Efremov <iefremov@unipro.ru> - 1.4.0-1
- Upstream version change

* Mon Feb 02 2009 Ivan Efremov <iefremov@unipro.ru> - 1.3.3-1
- Initial release of rpm