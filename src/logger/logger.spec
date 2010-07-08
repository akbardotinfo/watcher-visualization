%define	_project	cta
%define	_name		logger
%define	_prefix		/usr/%{_project}

Summary:	ARL1X/CTA
Name:		%{_project}-%{_name}
Version:	20100702
Release:	1
License:	Other
Group:		Development/CTA
URL:		http://fill.me.in/
Vendor:		SPARTA
Source:		%{name}-%{version}.tar.gz

BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Prefix:		%{_prefix}

BuildRequires:	automake
BuildRequires:	autoconf
BuildRequires:  pkgconfig
BuildRequires:	log4cxx-devel
BuildRequires:	libtool

%description

%prep
if [ "$RPM_BUILD_ROOT" ] && [ "$RPM_BUILD_ROOT" != "/" ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%setup -n %{_name}

%build
./autogen.sh
%configure
make DESTDIR=$RPM_BUILD_ROOT

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
if [ "$RPM_BUILD_ROOT" ] && [ "$RPM_BUILD_ROOT" != "/" ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%files
%defattr(-,root,root,-)

%{_includedir}/declareLogger.h
%{_includedir}/%{_name}.h
%{_libdir}/lib%{_name}.a
%{_libdir}/lib%{_name}.la
%{_libdir}/lib%{_name}.so
%{_libdir}/lib%{_name}.so.1
%{_libdir}/lib%{_name}.so.1.0.0
%{_libdir}/pkgconfig/%{_name}.pc

%changelog
