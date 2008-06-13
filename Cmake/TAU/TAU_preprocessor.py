#!/usr/bin/python

import sys, os

TAU_opt = []
OTHER_opt = []

TAU_cmp="@TAU_F90_COMPILER@"

cmp_found = False
last_opt  = False

for arg in sys.argv[1:len(sys.argv)]:
#     print """%s""" %(arg)
    if arg.find("=") != -1:
        tmp = arg[0:arg.find("=")]
        tmp += '=\"' + arg[arg.find("=")+1:len(arg)] + '\"'
        arg = tmp
    if arg.find("-opt") != -1:
        TAU_opt.append(arg)
        last_opt = True
    else:
        if last_opt and not cmp_found:
            cmp = arg
        else:
            OTHER_opt.append(arg)
        last_opt = False


cmd = TAU_cmp + " " + " ".join(TAU_opt) + " " + cmp + " " + " ".join(OTHER_opt)

print
print cmd 
print 

os.system(cmd)
