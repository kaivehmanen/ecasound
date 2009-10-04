#!/bin/bash
# 
# version:20091004-4
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

# specify ecasound binary used to generate test reference files
ECAS_REF=ecasound-2.6.0
CMP=../utils/ecacompare

set -x

# generate source file
$ECASOUND -q -f:16,1,44100 -i tone,sine,880,0 -o src44100.wav -t:10 || error_exit
ln -s src44100.wav src44100.foobar

# perform test 1
set -x
$ECAS_REF -q -f:16,1,44100 -i select,1,22000sa,src44100.wav -o as-dst22000sa-ref.wav -x || error_exit
$ECASOUND -q -f:16,1,44100 -i select,1,22000sa,src44100.wav -o as-dst22000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst22000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "22000" ] ; then error_exit ; fi
$CMP as-dst22000sa.wav as-dst22000sa-ref.wav
if [ $? != 0 ] ; then error_exit ; fi

# perform test 2 (lq -> ext resamplers are not necessarily supported)
set -x
$ECAS_REF -q -f:16,1,88200 -i select,1.9,33000sa,resample-lq,44100,src44100.wav -o as-dst33000sa-ref.wav -x || error_exit
$ECASOUND -q -f:16,1,88200 -i select,1.9,33000sa,resample-lq,44100,src44100.wav -o as-dst33000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst33000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "33000" ] ; then error_exit ; fi
$CMP as-dst33000sa.wav as-dst33000sa-ref.wav
if [ $? != 0 ] ; then error_exit ; fi

# perform test 3
set -x
$ECAS_REF -q -f:16,1,44100 -i select,40000sa,55000sa,typeselect,.wav,src44100.foobar -o as-dst55000sa-ref.wav -x || error_exit
$ECASOUND -q -f:16,1,44100 -i select,40000sa,55000sa,typeselect,.wav,src44100.foobar -o as-dst55000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst55000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "55000" ] ; then error_exit ; fi
$CMP as-dst55000sa.wav as-dst55000sa-ref.wav
if [ $? != 0 ] ; then error_exit ; fi

echo "Test run succesful (no manual verification needed)."
echo "Run './clean.sh' to remove created audio files."

exit 0
