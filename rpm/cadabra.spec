Summary: field-theory motivated computer algebra system
Name: cadabra
Version: 0.137
Release: 1
License: GPL
Group: Application/Math
Source: http://www.aei.mpg.de/~peekas/cadabra/cadabra-0.137.tar.gz
URL: http://www.aei.mpg.de/~peekas/cadabra
Packager: Kasper Peeters <kasper.peeters@aei.mpg.de>
BuildRoot: /tmp/cadabra-0.137
Prefix: /usr
Requires: tetex, lie, breqn

%description
Cadabra is a computer algebra system designed specifically for the
solution of problems encountered in field theory. It has extensive
functionality for tensor polynomial simplification including
multi-term symmetries, fermions and anti-commuting variables, Clifford
algebras and Fierz transformations, implicit coordinate dependence,
multiple index types and many more. The input format is a subset of
TeX.

%prep
%setup
./configure --prefix=/usr

%build
make

%files
/usr/bin/cadabra
/usr/bin/xcadabra
/usr/share/TeXmacs/plugins/cadabra/progs/init-cadabra.scm
/usr/share/man/man1/cadabra.1.gz
/usr/share/man/man1/xcadabra.1.gz
/usr/share/texmf/tex/latex/cadabra/tableaux.sty
/usr/share/texmf/tex/latex/cadabra/cadabra.sty
%docdir /usr/share/doc/cadabra
/usr/share/doc/cadabra

%install
make DESTDIR="$RPM_BUILD_ROOT" install

%post
texhash
