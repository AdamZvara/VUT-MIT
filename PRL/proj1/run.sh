#!/bin/bash

if [ $# -lt 1 ];then # pocet cisel bud zadam nebo 4 :)
    numbers=4;
else
    numbers=$1;
fi;

# pocet procesoru nastaven podle poctu cisel (lze i jinak)
proc=$(echo "(l($numbers)/l(2))+1" | bc -l | xargs printf "%1.0f") 

# preklad zdrojoveho souboru
# echo "Building app"
mpic++ -o pms pms.cpp

if [ $? -ne 0 ]; then
    exit 1
fi

#vyrobeni souboru s nahodnymi cisly
dd if=/dev/random bs=1 count=$numbers of=numbers 2> /dev/null	 

#spusteni programu
# echo "Running with $numbers nums and $proc processors"  
# echo "-------------------------------------------"
mpirun -np $proc --oversubscribe pms 				

#uklid
# rm -f pms numbers