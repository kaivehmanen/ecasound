#!/bin/bash
# 
# version:20080321-2
#
# Script to generate and test common resampling
# use cases. The output files need to be verified
# manually.
#
# ----------------------------------------------------------------------
# File: ecasound/manual-tests/test-resample.sh
# License: GPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

if test "x${ECASOUND}" = "x" ; then
  ECASOUND=../../ecasound/ecasound_debug
fi

# specify ecasound binary used to generate test reference files
ECAS_REF=ecasound
CMP=../utils/ecacompare

. test-common-sh

check_ecabin

set -x

# generate source file
$ECASOUND -q -f:16,1,96000 -b:1024 -i tone,sine,880,5 -o src96k.wav || error_exit
check_samples src96k.wav 480000

# perform resampling
$ECAS_REF -q -f:16,1,48000 -i resample,auto,src96k.wav -o re-dst48000-ref.wav -x || error_exit
$ECASOUND -q -f:16,1,48000 -i resample,auto,src96k.wav -o re-dst48000.wav -x || error_exit
check_zerosum re-dst48000-ref.wav re-dst48000.wav
$CMP re-dst48000.wav re-dst48000-ref.wav ; if [ $? != 0 ] ; then echo "Note: diff" ; fi
check_samples re-dst48000.wav 240000

$ECAS_REF -q -f:16,1,44100 -i resample,auto,src96k.wav -o re-dst44100-ref.wav -x || error_exit
$ECASOUND -q -f:16,1,44100 -i resample,auto,src96k.wav -o re-dst44100.wav -x || error_exit
check_zerosum re-dst44100-ref.wav re-dst44100.wav
#check_samples re-dst44100.wav 220450

$ECAS_REF -q -f:16,1,22050 -i resample,auto,src96k.wav -o re-dst22050-ref.wav -x || error_exit
$ECASOUND -q -f:16,1,22050 -i resample,auto,src96k.wav -o re-dst22050.wav -x || error_exit
check_zerosum re-dst22050-ref.wav re-dst22050.wav
#check_samples re-dst22050.wav 110250

$ECAS_REF -q -f:16,1,16000 -i resample,auto,src96k.wav -o re-dst16000-ref.wav -x || error_exit
$ECASOUND -q -f:16,1,16000 -i resample,auto,src96k.wav -o re-dst16000.wav -x || error_exit
check_zerosum re-dst16000-ref.wav re-dst16000.wav
#check_samples re-dst16000.wav 60000

$ECAS_REF -q -f:16,1,8000  -i resample,auto,src96k.wav -o re-dst8000-ref.wav -x || error_exit
$ECASOUND -q -f:16,1,8000  -i resample,auto,src96k.wav -o re-dst8000.wav -x || error_exit
check_zerosum re-dst8000-ref.wav re-dst8000.wav
#check_samples re-dst8000.wav 30000

echo "Test run succesful."
exit 0