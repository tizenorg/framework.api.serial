Name:       capi-network-serial
Summary:    Network Serial Framework
Version:    0.0.9
Release:    0
Group:      TO_BE/FILLED_IN
License:    Apache License, Version 2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(capi-base-common)

BuildRequires:  cmake


%description
Network Serial Framework

%package devel
Summary:    Network Serial Framework (DEV)
Group:      TO_BE/FILLED
Requires:   %{name} = %{version}-%{release}

%description devel
Network Serial Framework (DEV).

%prep
%setup -q

%build
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
install -D -m 0644 LICENSE %{buildroot}%{_datadir}/license/capi-network-serial
install -D -m 0644 LICENSE %{buildroot}%{_datadir}/license/capi-network-serial-devel

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest capi-network-serial.manifest
%defattr(-,root,root,-)
%{_libdir}/libcapi-network-serial.so.*
%{_datadir}/license/capi-network-serial

%files devel
%defattr(-,root,root,-)
%{_includedir}/network/serial.h
%{_libdir}/pkgconfig/capi-network-serial.pc
%{_libdir}/libcapi-network-serial.so
%{_datadir}/license/capi-network-serial-devel

%changelog
* Thu Aug 01 2013 Pavithra G <pavithra.g@samsung.com> 0.0.9-0
- TIZEN_ENGINEER_MODE is applied for dlog and securelog

* Thu Mar 28 2013 Injun Yang <injun.yang@samsung.com> 0.0.8-0
- Release dbus handler

* Wed Jan 9 2013 Injun Yang <injun.yang@samsung.com> 0.0.7-0
- Change the log tag of dlog

* Sun Nov 4 2012 Injun Yang <injun.yang@samsung.com> 0.0.6-1
- Fix the prevent issue

* Thu Oct 11 2012 Injun Yang <injun.yang@samsung.com> 0.0.5-3
- Add license file in rpm

* Tue Sep 25 2012 Injun Yang <injun.yang@samsung.com> 0.0.5
- Apply manifest
