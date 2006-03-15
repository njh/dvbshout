#!/bin/sh
# Run this to set up the build system: configure, makefiles, etc.

package="dvbshout"


srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

cd "$srcdir"
DIE=0

(autoheader --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have autoconf installed to compile $package."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have autoconf installed to compile $package."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have automake installed to compile $package."
    echo "Download the appropriate package for your system,"
    echo "or get the source from one of the GNU ftp sites"
    echo "listed in http://www.gnu.org/order/ftp.html"
    DIE=1
}

(pkg-config --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have pkg-config installed to compile $package."
    echo "Download the appropriate package for your system,"
    echo "or get the source from http://pkgconfig.freedesktop.org/"
    DIE=1
}


if test "$DIE" -eq 1; then
    exit 1
fi



echo "Generating configuration files for $package, please wait...."

run_cmd() {
    echo "  running $* ..."
    if ! $*; then
			echo failed!
			exit 1
    fi
}


run_cmd aclocal
run_cmd autoheader
run_cmd automake --add-missing --copy
run_cmd autoconf


$srcdir/configure && echo
