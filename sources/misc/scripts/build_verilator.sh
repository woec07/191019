#!/bin/bash

cd ../../../toolchain
INSTALL_PATH=$PWD/verilator

#Download
git clone https://github.com/verilator/verilator
unset VERILATOR_ROOT
cd verilator
git checkout tags/v5.030
autoconf
./configure --prefix $INSTALL_PATH
make -j8
