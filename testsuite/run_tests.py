#!/usr/bin/python

import sys
import os
import string

def check_for_test_data_files(filenamestr):
    try:
        file = open(filenamestr)
        file.close()
    except IOError:
        print "Test data file '" + filenamestr + "' not found; can't run tests. See 'README.txt'."
        sys.exit(1)

def run_tests():
    check_for_test_data_files("foo.wav")
    check_for_test_data_files("ecatestlist.txt")
    check_for_test_data_files("ecasound_test")
    
    file = open("ecatestlist.txt")
    lines = file.readlines()
    file.close()

    failed = 0
    for line in lines:
        testcases = string.split(line)
        for testcase in testcases:
            print "Running test " + testcase + ":"
            res = os.system("./" + testcase)
            print ""
            if res:
                failed = 1
    return failed

# main
sys.exit(run_tests())
        
