#!/bin/bash
source="../data/bun_zipper_res3.pts"

builders="loop"

valid=1

for grid in 64 128 ; do
    [ -e  res_ref_${grid}.pts ] ||  ./PMC --builder ref --grid $grid $source res_ref_${grid}.obj
    for builder in $builders; do
        ./PMC --builder $builder --grid $grid  $source res_${builder}_${grid}.obj

        echo -en "\e[36m"
        if python3 ../scripts/check_output.py res_${builder}_${grid}.obj res_ref_${grid}.obj ; then
            echo -n ""
        else
            echo -e "\e[31mInvalid output for builder $builder and grid $grid"
            valid=0
        fi
        echo -en "\e[0m"
    done
done

echo -n "Conclusion: "
if [ "$valid" -eq 1 ] ; then
    echo -e "\e[32mSuccess $valid \e[0m"
else
    echo -e "\e[31mOne or more tests failed\e[0m"
    exit 1
fi