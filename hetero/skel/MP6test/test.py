#!/usr/bin/python

import os

def result_check(fname):
    found = False
    text = "Solution is correct"
    f = file(fname)
    for line in f:
        if text in line:
            found = True
            break
    return found


os.system("cp ../../build/MP6 .")

#os.system("cp host-convol/conv .")

for i in range (0, 10):
    comstr = "./MP6 -e data/{0}/output.ppm -i data/{0}/input0.ppm,data/{0}/input1.csv -o out{0}.ppm -t image".format(i)
    #print comstr
    print "Running test: ", i
    fname = "data{0}.stdout".format(i)
    print "executing: ", comstr, " > ", fname
    os.system(comstr + " > " + fname)
    if (result_check(fname) == True):
        print "Correct"
    else:
        print "Failed: See execution log in: ", fname


