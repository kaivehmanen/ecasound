#!/bin/bash
# 
# version:20080706-3
#
# Script to generate and test audio selecting.
# The output files need to be verified manually.
#
# ----------------------------------------------------------------------
# File: ecasound/manual-tests/test-klg.sh
# License: GPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

if test "x${ECASOUND}" = "x" ; then
  ECASOUND=../../ecasound/ecasound_debug
fi

. test-common-sh

check_ecabin

set -x

# generate source file
$ECASOUND -q -f:16,1,44100 -i tone,sine,880,0 -o src44100.wav -t:10 || error_exit
ln -s src44100.wav src44100.foobar

# perform test 1
set -x
$ECASOUND -q -f:16,1,44100 -i select,1,22000sa,src44100.wav -o as-dst22000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst22000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "22000" ] ; then error_exit ; fi
check_md5sum as-dst22000sa.wav 6074b14f616a40cb1c7c74d305719c47

# perform test 2
set -x
$ECASOUND -q -f:16,1,88200 -i select,1.9,33000sa,resample,44100,src44100.wav -o as-dst33000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst33000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "33000" ] ; then error_exit ; fi
check_md5sum as-dst33000sa.wav 6118d58a9149a55f0392684b4c0fec81

# perform test 3
set -x
$ECASOUND -q -f:16,1,44100 -i select,40000sa,55000sa,typeselect,.wav,src44100.foobar -o as-dst55000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst55000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "55000" ] ; then error_exit ; fi

check_md5sum as-dst55000sa.wav 7ee71a96d9bfee811e61e11ade0523dd
# cur: 7ee71a96d9bfee811e61e11ade0523dd, size 110044
# prev-1: 0d792fe459a75e0e69e64c530d682fb3, size <unknown>

echo "Test run succesful (no manual verification needed)."
echo "Run './clean.sh' to remove created audio files."

exit 0
