#!/bin/sh
# 
# version:20080313-1 
#
# Script to generate and test audio selecting.
# The output files need to be verified manually.
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
$ECASOUND -q -f:16,1,44100 -i tone,sine,880,0 -o src44100.wav -t:10 || error_exit
ln -s src44100.wav src44100.foobar

# perform test 1
set -x
$ECASOUND -q -f:16,1,44100 -i audioselect,1,22000sa,src44100.wav -o as-dst22000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst22000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "22000" ] ; then error_exit ; fi

# perform test 2
set -x
$ECASOUND -q -f:16,1,88200 -i audioselect,1.9,33000sa,resample,44100,src44100.wav -o as-dst33000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst33000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "33000" ] ; then error_exit ; fi

# perform test 3
set -x
$ECASOUND -q -f:16,1,44100 -i audioselect,40000sa,55000sa,typeselect,.wav,src44100.foobar -o as-dst55000sa.wav -x || error_exit
set +x
samples=`sndfile-info as-dst55000sa.wav |grep Frames |cut -d ':' -f2`
if [ $samples != "55000" ] ; then error_exit ; fi

echo "Test run succesful (no manual verification needed)."
echo "Run './clean.sh' to remove created audio files."

exit 0