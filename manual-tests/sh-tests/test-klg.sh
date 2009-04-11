#!/bin/bash
# 
# version:20080708-3
#
# Script to generate and test common resampling
# use cases. The output files need to be verified
# manually.
#
# ----------------------------------------------------------------------
# File: ecasound/manual-tests/test-klg.sh
# License: GPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

if test "x${ECASOUND}" = "x" ; then
  ECASOUND=../../ecasound/ecasound_debug
fi

# whether to skip md5sum checks
SKIP_MD5SUM=0

. test-common-sh

check_ecabin

set -x

# control amplify with klg
#   0.0  -> 5.5:   0%   -> 100%
#   5.5  -> 10.5:  100% -> 20%
#   10.5 -> 20.0:  20%  -> 80%
#   20.0 -> 30.0:  80%  -> 10%
#
$ECASOUND -q -f:16,1,44100 -i tone,sine,440,30 -o klg-dst.wav -ea:100 -klg:1,0,100,5,0,0,5.5,1,10.5,0.2,20.0,0.8,30,0.1 -x  || error_exit
check_1dbpeak_count klg-dst.wav 5753
check_filesize klg-dst.wav 2646042
# cur...: e7c1a4d352423eb9c40f184274546c8e, size 2646042
# prev-2: e4a87025d0205016cb3c9f7e6b5aa63f, size 2646042
# prev-1: e7c1a4d352423eb9c40f184274546c8e, size <unknown>

echo "Test run succesful."
exit 0
