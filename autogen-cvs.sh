#!/bin/sh

# explicitly select autoconf version to use
ECA_AM_VERSION=""
#ECA_AM_VERSION="-1.9"

# explicitly select autoconf version to use
ECA_AC_VERSION=""
#ECA_AC_VERSION="2.50"

# created variables for tools
export AUTOMAKE=automake${ECA_AM_VERSION}
export ACLOCAL=aclocal${ECA_AM_VERSION}
export AUTOHEADER=autoheader${ECA_AC_VERSION}
export AUTOCONF=autoconf${ECA_AC_VERSION}

# test for old versions
version_tmp=`$AUTOMAKE --version |head -1 |cut -d ' ' -f 4 |head -c 3`
version_maj=`echo $version_tmp |cut -d . -f 1`
version_min=`echo $version_tmp |cut -d . -f 2`
echo -e "Detected automake version ${version_maj}.${version_min}.\n"

if [ $version_maj = "1" -a $((version_min < 6)) = "1" ] ; then
   echo "Error: automake-1.6 or newer required. "
   echo "Edit the ECA_AM_VERSION variable in this file or install "
   echo "a newer automake package as the system default." 
   exit 1
fi

libtoolize --copy --automake

$ACLOCAL
$AUTOHEADER
$AUTOMAKE --add-missing --force-missing --copy
$AUTOCONF

if test x$1 != x--no-conf; then
    ./configure $@ || exit 1
fi
