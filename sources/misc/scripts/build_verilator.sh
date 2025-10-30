#!/bin/bash

if [ ! -d /opt/hwe ]; then
    echo "opt/hwe directory doesnt exist.  Creating it"
    mkdir /opt/hwe
fi
cd /opt/hwe/
INSTALL_PATH=$PWD/verilator

#Download
sudo git clone https://github.com/verilator/verilator verilator_source
unset VERILATOR_ROOT
cd verilator_source
sudo git checkout tags/v5.030
sudo autoconf
sudo ./configure --prefix $INSTALL_PATH
sudo make -j8
sudo make install 
sudo cp -r  /opt/hwe/verilator_source/include/ /opt/hwe/verilator/
