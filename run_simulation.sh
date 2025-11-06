#!/bin/bash

#Top level script to build verilator model, build all test programs, and run all test programs.  


if [ $# -ne 1 ]
then
    echo "Usage: ./run_simulation.sh TEST_NAME"
    exit 1
fi

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

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to Build Verilator Model.  Check your syntax and configuration variables in vector_config.cmake"
    exit
fi

make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to Build Verilator Model.  Errors should be caught earlier than this.  If you see this error, try deleting the /sources/rtl/build directory and building again."
    exit
fi

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

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build C Programs.  Check your syntax your vector code C files."
    exit
fi

ctest -R $1


######
# Display CTest Report
######

cat $PWD/Testing/Temporary/LastTest.log
