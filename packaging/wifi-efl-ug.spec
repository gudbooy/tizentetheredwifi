Name:		wifi-efl-ug
Summary:	Wi-Fi UI Gadget for TIZEN
Version:	1.0.105
Release:	1
Group:		App/Network
License:	Flora License
Source0:	%{name}-%{version}.tar.gz
BuildRequires:	pkgconfig(ecore)
BuildRequires:	pkgconfig(ecore-imf)
BuildRequires:	pkgconfig(ecore-input)
BuildRequires:	pkgconfig(appcore-efl)
BuildRequires:	pkgconfig(elementary)
BuildRequires:	pkgconfig(efl-assist)
BuildRequires:	pkgconfig(glib-2.0)
BuildRequires:	pkgconfig(openssl)
BuildRequires:	pkgconfig(cert-svc-vcore)
BuildRequires:	pkgconfig(utilX)
BuildRequires:	pkgconfig(ui-gadget-1)
BuildRequires:	pkgconfig(x11)
BuildRequires:	pkgconfig(sensor)
BuildRequires:	pkgconfig(capi-network-wifi)
BuildRequires:	pkgconfig(capi-network-bluetooth)
BuildRequires:	pkgconfig(capi-network-connection)
BuildRequires:	pkgconfig(capi-network-tethering)
BuildRequires:	pkgconfig(network)
BuildRequires:	pkgconfig(feedback)
BuildRequires:	pkgconfig(capi-system-info)
#BuildRequires:  pkgconfig(setting-common-internal)
#BuildRequires:  pkgconfig(setting-lite-common-internal)
BuildRequires:	cmake
BuildRequires:	gettext-tools
BuildRequires:	edje-tools
BuildRequires:	model-build-features
Requires(post):		/sbin/ldconfig
Requires(post):		/usr/bin/vconftool
requires(postun):	/sbin/ldconfig

%description
Wi-Fi UI Gadget

%package -n net.wifi-qs
Summary:    Wi-Fi System popup
Requires:   %{name} = %{version}
Requires(post):   /usr/bin/vconftool

%description -n net.wifi-qs
Wi-Fi System popup for TIZEN

%prep
%setup -q

%define PREFIX /usr/


%build
#LDFLAGS+="-Wl,--rpath=%{PREFIX}/lib -Wl,--as-needed"
cmake -DCMAKE_INSTALL_PREFIX=%{PREFIX} \
%if ! 0%{?model_build_feature_network_tethering_disable}
	-DTIZEN_TETHERING_ENABLE=1 \
%endif
	.

make %{?_smp_mflags}


%install
%make_install

mkdir -p %{buildroot}%{_sysconfdir}/smack/accesses.d/
cp -v net.wifi-qs.efl %{buildroot}%{_sysconfdir}/smack/accesses.d/

#License
mkdir -p %{buildroot}%{_datadir}/license
cp LICENSE %{buildroot}%{_datadir}/license/wifi-efl-ug
cp LICENSE %{buildroot}%{_datadir}/license/net.wifi-qs

%post
/sbin/ldconfig

mkdir -p %{PREFIX}/bin/

vconftool set -t int memory/wifi/ug_run_state 3 -i -g 6519 -s system::vconf_setting

vconftool set -t int memory/wifi/wifi_qs_exit 0 -g 6519 -i -s system::vconf_inhouse
vconftool set -t int db/wifi/enable_quick_start 1 -g 6519 -i -s system::vconf_setting

vconftool set -t int file/private/wifi/network_bonding 0 -g 6519 -s system::vconf_setting
vconftool set -t int file/private/wifi/sort_by 1 -g 6519 -s system::vconf_setting

%postun -p /sbin/ldconfig


%files
%manifest wifi-efl-ug.manifest
%{PREFIX}/apps/wifi-efl-ug/lib/ug/*
%attr(644,-,-) %{PREFIX}/apps/wifi-efl-ug/lib/*
%attr(755,-,-) %{PREFIX}/apps/wifi-efl-ug/lib/ug
%{PREFIX}/apps/wifi-efl-ug/res/edje/wifi-efl-UG/*.edj
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_datadir}/license/wifi-efl-ug
%{_datadir}/packages/wifi-efl-ug.xml
/usr/apps/wifi-efl-ug/shared/res/tables/ug-wifi-efl_ChangeableColorTable.xml
/usr/apps/wifi-efl-ug/shared/res/tables/ug-wifi-efl_FontInfoTable.xml

%files -n net.wifi-qs
%manifest net.wifi-qs.manifest
%{_bindir}/wifi-qs
%{_datadir}/packages/net.wifi-qs.xml
%{_datadir}/icons/*.png
%{PREFIX}/apps/wifi-efl-ug/res/edje/wifi-qs/*.edj
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_datadir}/license/net.wifi-qs
%{_sysconfdir}/smack/accesses.d/net.wifi-qs.efl
