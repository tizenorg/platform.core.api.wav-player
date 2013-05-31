Name:       capi-media-wav-player
Summary:    A wav player library in Tizen C API
Version: 0.1.0
Release:    16
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-sound-manager)

%description
A wav player library in Tizen C API.


%package devel
Summary:  A wav player library in Tizen C API (Development)
Group:    Development/Multimedia
Requires: %{name} = %{version}-%{release}

%description devel
%devel_desc



%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%license LICENSE.APLv2
%{_libdir}/libcapi-media-wav-player.so.*
%manifest capi-media-wav-player.manifest

%files devel
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-wav-player.so
