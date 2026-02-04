#!/bin/bash
file=$1
src=$(basename "$file")

scp $file riscv@203.0.113.2:/home/riscv

ssh riscv@203.0.113.2 "g++ -fopenmp $src -o $src.o && ./$src.o"
