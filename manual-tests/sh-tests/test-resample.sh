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

SKIP_MD5SUM=1

. test-common-sh

check_ecabin

set -x

# generate source file
$ECASOUND -q -f:16,1,96000 -i tone,sine,880,0 -o src96k.wav -t:5 || error_exit

# perform resampling
$ECASOUND -q -f:16,1,48000 -i resample,auto,src96k.wav -o re-dst48000.wav -x || error_exit
check_md5sum re-dst48000.wav 41a6a6bbdf41fde8026d43cfa21fa804
$ECASOUND -q -f:16,1,44100 -i resample,auto,src96k.wav -o re-dst44100.wav -x || error_exit
check_md5sum re-dst44100.wav e763f450cbd21dd9dca536d5693dd395
$ECASOUND -q -f:16,1,22050 -i resample,auto,src96k.wav -o re-dst22050.wav -x || error_exit
check_md5sum re-dst22050.wav 9eab303aa8657fe1b505721bf2810948
$ECASOUND -q -f:16,1,16000 -i resample,auto,src96k.wav -o re-dst16000.wav -x || error_exit
check_md5sum re-dst16000.wav 4629e0fedd434d1be0eaf596ca9eec1b
$ECASOUND -q -f:16,1,8000  -i resample,auto,src96k.wav -o re-dst8000.wav -x || error_exit
check_md5sum re-dst8000.wav e6d715a18d3850781e6abcf96196a54d

echo "Test run succesful."
exit 0