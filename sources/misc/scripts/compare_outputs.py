import numpy
import os
import sys


file_ref = open(sys.argv[1], 'r')
file_test = open(sys.argv[2], 'r')

ref_cycles, ref_instr, test_cycles, test_instr = 0,0,0,0


for line in file_ref:
    if "Cycles" in line:
        splitline = line.split()
        ref_cycles = splitline[-1]
    if "Instructions" in line:
        splitline = line.split()
        ref_instr = splitline[-1]

for line in file_test:
    if "Cycles" in line:
        splitline = line.split()
        test_cycles = splitline[-1]
    if "Instructions" in line:
        splitline = line.split()
        test_instr = splitline[-1]




perc_cycles = ((float(ref_cycles) - float(test_cycles)) / float(ref_cycles)) * 100
perc_instr = ((float(ref_instr) - float(test_instr)) / float(ref_instr)) * 100
print("\nReference Cycles: " + str(ref_cycles))
print("Vector Cycles:    " + str(test_cycles))

print(str(perc_cycles) + "% improvement\n")
if (perc_cycles < 0):
    print("WARNING: Vector configuration performs worse\n")

print("Reference Instr: " + str(ref_instr))
print("Vector Instr:    " + str(test_instr))
print(str(perc_instr) + "% improvement")