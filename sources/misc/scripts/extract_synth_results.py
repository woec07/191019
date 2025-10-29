import numpy
import os
import sys

file_report = open(sys.argv[1], 'r')

for line in file_report:
    if "| Slice LUTs                 |" in line:
        splitline = line.split("|")
        LUT = splitline[2]
        print("LUT Usage : " + LUT)
        exit
    if "IO Placement failed due to overutilization" in line:
        print("VMEM_W Port is too large, not enough I/O for this design.  Reduce the size of this port to allow for successful synthesis.")
        exit



print("Synthesis Result not found.  Check the synthesis_log.txt for any errors.")
