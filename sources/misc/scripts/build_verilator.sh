#!/bin/bash

if [ ! -d /opt/hwe ]; then
    echo "opt/hwe directory doesnt exist.  Creating it"
    mkdir /opt/hwe
fi
cd /opt/hwe
INSTALL_PATH=$PWD/verilator

#Download
git clone https://github.com/verilator/verilator
unset VERILATOR_ROOT
cd verilator
git checkout tags/v5.030
autoconf
sudo ./configure --prefix $INSTALL_PATH
sudo make -j8
