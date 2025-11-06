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
if [[ "$?" != "0" ]]; then
    echo -e "\033[91m compilation failed!!!! \033[0m"
    exit 1
fi

ctest -R $1


######
# Display CTest Report
######

cat $PWD/Testing/Temporary/LastTest.log
