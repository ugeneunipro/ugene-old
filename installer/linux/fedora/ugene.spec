Name:    ugene
Summary: Integrated bioinformatics toolkit
Version: 1.16.0
Release: 1%{?dist}
#The entire source code is GPLv2+ except:
#file src/libs_3rdparty/qtbindings_core/src/qtscriptconcurrent.h which is GPLv2
#files in src/plugins_3rdparty/script_debuger/src/qtscriptdebug/ which are GPLv2
License: GPLv2+ and GPLv2
Group:   Applications/Engineering
URL:     http://ugene.unipro.ru
Source0: http://ugene.unipro.ru/downloads/%{name}-%{version}.tar.gz

BuildRequires: qt4-devel pkgconfig(QtWebKit) zlib-devel desktop-file-utils procps-devel qt-mysql
#We need strict versions of qt for correct work of src/libs_3rdparty/qtbindings_*
%{?_qt4_version:Requires: qt4%{?_isa} >= %{_qt4_version}}
Provides: bundled(sqlite)
Provides: bundled(samtools)
ExclusiveArch: %{ix86} x86_64

%description
Unipro UGENE is a cross-platform visual environment for DNA and protein
sequence analysis. UGENE integrates the most important bioinformatics
computational algorithms and provides an easy-to-use GUI for performing
complex analysis of the genomic data. One of the main features of UGENE
is a designer for custom bioinformatics workflows.

%prep
%setup -q

%build
%_bindir/qmake-qt4 -r -spec linux-g++ \
        INSTALL_BINDIR=%{_bindir} \
        INSTALL_LIBDIR=%{_libdir} \
        INSTALL_DATADIR=%{_datadir} \
        INSTALL_MANDIR=%{_mandir} \
%if 0%{?_ugene_with_non_free}
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
%{_datadir}/icons/*
%{_datadir}/mime/*
%{_datadir}/%{name}/
%{_mandir}/man1/*
%doc COPYRIGHT LICENSE LICENSE.3rd_party 

%changelog
* Wed Feb 25 2015 Yuliya Algaer <yalgaer@unipro.ru> - 1.16.0-1
- New upstream version

* Wed Jan 14 2015 Yuliya Algaer <yalgaer@unipro.ru> - 1.15.1-1
- New upstream version

* Tue Dec 2 2014 Yuliya Algaer <yalgaer@unipro.ru> - 1.15.0-1
- New upstream version

* Fri Oct 10 2014 Yuliya Algaer <yalgaer@unipro.ru> - 1.14.2-2
- Minor fix of the spec file

* Fri Oct 10 2014 Yuliya Algaer <yalgaer@unipro.ru> - 1.14.2-1
- New upstream release

* Thu Sep 11 2014 Yuliya Algaer <yalgaer@unipro.ru> - 1.14.1-1
- New upstream release

* Mon Aug 18 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.14.0-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Thu Aug 07 2014 Yuliya Algaer <yalgaer@unipro.ru> - 1.14.0-4
- Minor fix

* Tue Aug 05 2014 Yuliya Algaer <yalgaer@unipro.ru> - 1.14.0-3
- Minor fix

* Tue Aug 05 2014 Yuliya Algaer <yalgaer@unipro.ru> - 1.14.0-2
- Minor fix

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.13.3-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Tue Jun 03 2014 Yuliya Algaer <yalgaer@unipro.ru> 1.13.3-1
- Upstream version change

* Mon Apr 14 2014 Yuliya Algaer <yalgaer@unipro.ru> 1.13.2-1
- Upstream version change

* Sat Mar 08 2014 Kevin Kofler <Kevin@tigcc.ticalc.org> 1.13.1-2
- Rebuild against fixed qt to fix -debuginfo (#1074041)

* Thu Feb 20 2014 Yulia Algaer <yalgaer@unipro.ru> 1.13.1-1
- Upstream version change

* Wed Dec 11 2013 Yulia Algaer <yalgaer@unipro.ru> 1.13.0-1
- Upstream version change

* Wed Oct 09 2013 Yulia Algaer <yalgaer@unipro.ru> 1.12.3-1
- Upstream version change

* Mon Aug 19 2013 Yulia Algaer <yalgaer@unipro.ru> 1.12.2-1
- Upstream version change

* Wed Aug 07 2013 Yulia Algaer <yalgaer@unipro.ru> 1.12.1-1
- Upstream version change

* Sun Aug 04 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.12.0-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Fri Jun 28 2013 Yulia Algaer <yalgaer@unipro.ru> 1.12.0-2
- Minor fix

* Fri Jun 28 2013 Yulia Algaer <yalgaer@unipro.ru> 1.12.0-1
- Upstream version change

* Tue Mar 12 2013 Yulia Algaer <yalgaer@unipro.ru> 1.11.5-2
- Fix version typo

* Tue Mar 12 2013 Yulia Algaer <yalgaer@unipro.ru> 1.11.5-1
- Upstream version change

* Tue Jan 22 2013 Yulia Algaer <yalgaer@unipro.ru> 1.11.4-1
- Upstream version change

* Tue Nov 27 2012 Rex Dieter <rdieter@fedoraproject.org> 1.11.3-2
- fix/update qt-related dependencies

* Fri Nov 2 2012 Yulia Algaer <yalgaer@unipro.ru> - 1.11.3-1 
- Upstream version change

* Wed Oct 3 2012 Yulia Algaer <yalgaer@unipro.ru> - 1.11.2-1 
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