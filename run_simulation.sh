#!/bin/bash

#Top level script to build verilator model, build all test programs, and run all test programs.  



######
# Build Verilator Model
######
if [ -d $PWD/sources/rtl/build ]; then
    echo "Build Directory already exists.  Cleaning it up."
    cd $PWD/sources/rtl/build
    make clean
else
    echo "Build Directory doesn't exist.  Creating one"
    mkdir $PWD/sources/rtl/build
    cd $PWD/sources/rtl/build
fi

cmake ..

make -j$(nproc)

cd ../../..


######
# Build Programs
######
if [ -d $PWD/sources/programs/build ]; then
    echo "Build Directory already exists.  Cleaning it up."
    cd $PWD/sources/programs/build
    make clean
else
    echo "Build Directory doesn't exist.  Creating one"
    mkdir $PWD/sources/programs/build
    cd $PWD/sources/programs/build
fi

cmake ..

make -j$(nproc)

ctest


######
# Display CTest Report
######

cat $PWD/Testing/Temporary/LastTest.log
