#!/bin/bash
# 
# version:20091004-4
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

# specify ecasound binary used to generate test reference files
ECAS_REF=ecasound-2.6.0
CMP=../utils/ecacompare

. test-common-sh

check_ecabin

set -x

# control amplify with klg
#   0.0  -> 5.5:   0%   -> 100%
#   5.5  -> 10.5:  100% -> 20%
#   10.5 -> 20.0:  20%  -> 80%
#   20.0 -> 30.0:  80%  -> 10%
#
$ECAS_REF -q -f:16,1,44100 -b:1024 -i tone,sine,440,30 -o klg-dst-ref.wav -ea:100 -klg:1,0,100,5,0,0,5.5,1,10.5,0.2,20.0,0.8,30,0.1 -x  || error_exit
$ECASOUND -q -f:16,1,44100 -b:1024 -i tone,sine,440,30 -o klg-dst.wav -ea:100 -klg:1,0,100,5,0,0,5.5,1,10.5,0.2,20.0,0.8,30,0.1 -x  || error_exit
#check_1dbpeak_count klg-dst.wav 5756
check_filesize klg-dst.wav 2646044
check_samples klg-dst.wav 1323000
set -x
$CMP klg-dst-ref.wav klg-dst.wav
if [ $? != 0 ] ; then 
  set +x
  echo "NOTE: ecacompare returned failure, manual verification recommended."
fi

set +x 
echo "Test run succesful."
echo "Run './clean.sh' to remove created audio files."

exit 0
