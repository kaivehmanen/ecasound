=======================================================================
*** Ecasound Testsuite - README.txt                                 ***
=======================================================================

---
General

This directory contains a set of small programs that 
test various parts of ecasound.

---
Test programs

All test programs are standalone applications that either 
return 0 (for success), or non-zero (for error).

Issue './run_tests.py' to run the whole test suite.

---
Test data files

Most tests are performed using ecasound's 'null' and 
'rtnull' audio objects. Howver, some tests require
real audio objects. In these cases, the following 
files and device are used:

./ecasound_test	  - a symbolic link to ecasound executable
./foo.wav	  - generic input wav-file
./bigfoo.wav	  - a big (>10MB) input file
/dev/dsp	  - OSS output file

---
List of test categories

CON - Test cases for testing the 'ecasound' console 
      interface. 
ECI - Test cases utilizing the ECI API (Ecasound Control 
      Interface); (eci_*)

---
List of current tests

CON-1 - Simple tests for command-line options and basic 
        operations.
CON-2 - Tests for various rt and nonrt object combinations.
        Should be run both with and without root-priviledges.

ECI-1 - Initializing the ECI C-interface multiple times.
ECI-2 - Like ECI-1, but uses re-entrant API functions.
ECI-3 - Snapshot test for basic ECI C API functionality, where 
            a simple chainsetup is configured, connected and then
	    executed. Multiple error conditions.


-----------------------------------------------------------------------
