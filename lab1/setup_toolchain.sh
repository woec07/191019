#!/bin/bash

#Top level script to setup dependencies/toolchain.  Each has a helper script to setup which is called here

#Necessary dependencies:
#   -verilator:  verilator is built from source.
#   -GCC      :  RISCV GCC headers are needed for each supported architecture.


git submodule update --init --recursive

######
#   Verilator setup
######

./sources/misc/scripts/build_verilator.sh

######
#   GCC setup
######

./sources/misc/scripts/build_gcc.sh rv32im_zve32x ilp32

######
#   Download Srecord
######

cd /opt/hwe
sudo wget -O srec.tar.gz https://sourceforge.net/projects/srecord/files/srecord/1.65/srecord-1.65.0-Linux.tar.gz
sudo tar -xvzf srec.tar.gz
sudo mv srecord-1.65.0-Linux/ srec/

