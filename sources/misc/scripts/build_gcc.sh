#!/bin/bash

# Arguments: $1 - architecture for gcc
#            $2 - ABI
if [ ! -d /opt/hwe ]; then
    echo "opt/hwe directory doesnt exist.  Creating it"
    mkdir /opt/hwe
fi
cd /opt/hwe

INSTALL_PATH=$PWD/GCC/$1

if [ ! -d $PWD/GCC ]; then
    echo "/opt/hwe/GCC directory doesnt exist.  Creating it"
    sudo mkdir GCC 
fi

#Build GCC
echo "Downloading GCC"
if [ -d $PWD/riscv-gnu-toolchain ]; then
    echo "GCC source already downloaded. Cleaning it up"
    cd riscv-gnu-toolchain
    make clean
else
    git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git
    cd riscv-gnu-toolchain
fi

echo "making GCC for $1 $2"
sudo ./configure --prefix=$INSTALL_PATH --with-arch=$1 --with-abi=$2
sudo make -j8