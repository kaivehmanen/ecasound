#!/bin/bash
# 
# version:20080324-1
#
# Script to generate and test common resampling
# use cases. The output files need to be verified
# manually.
#
# ----------------------------------------------------------------------
# File: ecasound/manual-tests/test-rtnull.sh
# License: GPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

# whether to skip md5sum checks
SKIP_MD5SUM=0

. test-common-sh

set_ecasound_envvar
check_ecabin

t=`( time -p $TESTECASOUND -q -f:16,1,44100 -i rtnull -o null -t:5.0 -z:nodb -z:nointbuf -b:128 2>/dev/null ) 2>&1 |grep real | cut -f2 -d' ' |cut -f1 -d'.'`

# test is succesful if the duration is 5.x or 6.x seconds 
# (the duration varies somewhat due to process startup delays, etc,
# and is thus not exact)
test "x$t" = "x6" || test "x$t" = "x5" || error_exit

echo "Test run succesful."
exit 0