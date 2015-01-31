%define APP_PREFIX %{_prefix}/apps/org.tizen.w-text-input-setting
%define APP_DATADIR /opt/usr/apps/org.tizen.w-text-input-setting/data

Name: org.tizen.w-text-input-setting
Summary: w-text-input-setting
Version: 0.0.1
Release: 1
Group: HomeTF/Homescreen
License: Apache License, Version 2.0
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake, gettext, edje-bin, coreutils
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(isf)
BuildRequires: efl-assist-devel
BuildRequires: pkgconfig(vconf-internal-keys)
BuildRequires: pkgconfig(vconf)
Requires(post):/usr/bin/vconftool

%description
w-text-input-setting

%prep
%setup -q

%build
export CFLAGS="${CFLAGS} -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="${CXXFLAGS} -DTIZEN_ENGINEER_MODE"
export FFLAGS="${FFLAGS} -DTIZEN_ENGINEER_MODE"
CFLAGS+=" -fPIC -pie -rdynamic -Winline"
CXXFLAGS+=" -fPIC -pie -rdynamic -Winline"
LDFLAGS+="-Wl,--rpath=/usr/lib -Wl,--as-needed -lgcc_s";export LDFLAGS

rm -rf CMakeFiles
rm -rf CMakeCache.txt
cmake . -DCMAKE_INSTALL_PREFIX=%{APP_PREFIX}
make %{?jobs:-j%jobs}

%install
%make_install
%define tizen_sign 1
%define tizen_sign_base /usr/apps/org.tizen.w-text-input-setting
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1
mkdir -p %{buildroot}%{APP_DATADIR}


%post
%postun

%files
%manifest org.tizen.w-text-input-setting.manifest
%defattr(-,root,root,-)
%attr(-,app,app) %dir %{APP_DATADIR}
%{_datarootdir}/packages/org.tizen.w-text-input-setting.xml
%{_sysconfdir}/smack/accesses.d/*
%{_datarootdir}/license/*
%{APP_PREFIX}/*
