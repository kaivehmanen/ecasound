#!/bin/sh
#
# version:20080819-3

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

autoreconf --install --force --verbose

# test for --reconf
if test x$1 = "x--reconf"; then
    ./config.status --recheck && ./config.status
fi
