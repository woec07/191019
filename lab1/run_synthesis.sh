#!/bin/bash

#Top level script to run synthesis  


if [ $# -ne 0 ]
then
    echo "Usage: ./run_synthesis.sh"
    exit 1
fi

export TOP=$PWD

######
# Run synthesis
######
echo "Beginning Synthesis"
cd $PWD/sources/synthesis/
vivado -mode Tcl -source synth_commands.tcl &> synthesis_log.txt

######
# Extract Results
######
python3 $TOP/sources/misc/scripts/extract_synth_results.py $TOP/sources/synthesis/synthesis_log.txt

