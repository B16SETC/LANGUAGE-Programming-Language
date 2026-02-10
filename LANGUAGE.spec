Name:           LANGUAGE
Version:        0.1.0
Release:        1%{?dist}
Summary:        A simple, minimalist programming language

License:        MIT
URL:            https://github.com/yourusername/LANGUAGE
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake >= 3.20

%description
LANGUAGE is a simple, minimalist programming language built one feature at a time.
Features include variable assignment, arithmetic operations, and print statements.

%prep
%autosetup

%build
mkdir -p build
cd build
%cmake ..
%cmake_build

%install
cd build
install -D -m 0755 redhat-linux-build/LANGUAGE %{buildroot}%{_bindir}/LANGUAGE

%files
%{_bindir}/LANGUAGE

%changelog
* Mon Feb 10 2025 Your Name <your.email@example.com> - 0.1.0-1
- Initial release
- Day 1: Variables, arithmetic, and Print statements
