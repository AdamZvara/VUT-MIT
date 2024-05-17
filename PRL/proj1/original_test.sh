#!/bin/bash

if [ $# -lt 1 ];then # pocet cisel bud zadam nebo 4 :)
    numbers=4;
else
    numbers=$1;
fi;

# pocet procesoru nastaven podle poctu cisel (lze i jinak)
proc=$(echo "(l($numbers)/l(2))+1" | bc -l | xargs printf "%1.0f") 

# preklad zdrojoveho souboru
mpic++ -o pms pms.cpp

#vyrobeni souboru s nahodnymi cisly
dd if=/dev/random bs=1 count=$numbers of=numbers 2> /dev/null	 

#spusteni programu
mpirun --prefix /usr/local/share/OpenMPI  -np $proc pms 				

#uklid
# rm -f pms numbers