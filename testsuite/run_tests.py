#!/usr/bin/python

import os
import string

file = open("ecatestlist.txt")
lines = file.readlines()
file.close()

for line in lines:
    testcases = string.split(line)
    for testcase in testcases:
        print "Running test " + testcase + ":"
        os.system("./" + testcase)
    
