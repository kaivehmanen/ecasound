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

./foo.wav	  - generic input wav-file
./bigfoo.wav	  - a big (>10MB) input file
/dev/dsp	  - OSS output file

---
List of subsystem tags

[ECI] - Ecasound Control Interface (eci_*)

---
List of current tests

ECI-1 - Initializing the ECI C-interface multiple times.
ECI-2 - Like ECI-1, but uses re-entrant API functions.
ECI-3 - Snapshot test for basic ECI C API functionality, where 
        a simple chainsetup is configured, connected and then
	executed. Multiple error conditions.

-----------------------------------------------------------------------
