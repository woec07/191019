#!/bin/bash

#Top level script to setup dependencies/toolchain.  Each has a helper script to setup which is called here
#
#Necessary dependencies:
#   -verilator:  verilator is built from source.
#   -GCC      :  RISCV GCC headers are needed for each supported architecture.



######
# make Toolchain directory
######
if [ -d $PWD/toolchain ]; then
    echo "Toolchain Directory already exists"
else
    echo "Making Toolchain Directory"
    mkdir $PWD/toolchain
fi
######
#   Verilator setup
######

./sources/misc/scripts/build_verilator.sh

######
#   GCC setup
######
./sources/misc/scripts/build_gcc.sh rv32im ilp32

./sources/misc/scripts/build_gcc.sh rv32im_zve32x ilp32
