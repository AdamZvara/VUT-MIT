#!/bin/bash

# Author: Adam Zvara (xzvara01@stud.fit.vutbr.cz)
# Date: 04/2024

if [ -z "$1" ]; then
    echo "Provide input grid file"
    exit 1
fi

if [ -z "$2" ]; then
    echo "Provide number of iterations"
    exit 1
fi

# get number of (non-empty) lines in file: https://stackoverflow.com/a/114861
lines=$(grep -cve '^\s*$' $1)

# get number of physical cores: https://stackoverflow.com/a/6481016/21351640
proc=$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')

# if number of lines is less than default number of processes, use number of lines
if [ $lines -lt $proc ]; then
    proc=$lines
fi

mpic++ -o life life.cpp
mpirun --prefix /usr/local/share/OpenMPI -np $proc life $1 $2