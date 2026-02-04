#!/bin/bash
set -e
hostname=$1
src=$2

chmod +x connect_bpi3.sh

echo "============= Connect BPI-F3==================="

rpa_shell -c $hostname "./multicore_lab/connect_bpi3.sh ./multicore_lab/$src"
