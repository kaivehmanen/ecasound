#!/bin/sh
# 
# version:20080313-1 
#
# Script to generate and test common resampling
# use cases. The output files need to be verified
# manually.
#


ECASOUND=../ecasound/ecasound_debug

function error_exit() {
  echo "ERROR: Test failure, exiting."
  exit 1
}

if [ ! -e $ECASOUND ] ; then
  echo "Ecasound binary not found."
fi

set -x

# generate source file
$ECASOUND -q -f:16,1,96000 -i tone,sine,880,0 -o src96k.wav -t:5 || error_exit

# perform resampling
$ECASOUND -q -f:16,1,48000 -i resample,auto,src96k.wav -o re-dst48000.wav -x || error_exit
$ECASOUND -q -f:16,1,44100 -i resample,auto,src96k.wav -o re-dst44100.wav -x || error_exit
$ECASOUND -q -f:16,1,22050 -i resample,auto,src96k.wav -o re-dst22050.wav -x || error_exit
$ECASOUND -q -f:16,1,16000 -i resample,auto,src96k.wav -o re-dst16000.wav -x || error_exit
$ECASOUND -q -f:16,1,8000  -i resample,auto,src96k.wav -o re-dst8000.wav -x || error_exit

echo "Test run succesful."
exit 0