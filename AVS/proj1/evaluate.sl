#!/bin/bash
#SBATCH -p qcpu_exp
#SBATCH -A DD-23-135
#SBATCH -n 1 
#SBATCH -t 0:25:00
#SBATCH --mail-type END
#SBATCH -J AVS-evaluate


cd $SLURM_SUBMIT_DIR
ml intel-compilers/2022.1.0 CMake/3.23.1-GCCcore-11.3.0 # pouze na Barbore

[ -d build_evaluate ] && rm -rf build_evaluate
[ -d build_evaluate ] || mkdir build_evaluate

cd build_evaluate
rm tmp_*

CC=icc CXX=icpc cmake ..
make


SHAPES=(512 1024 2048 4096)
ITERS=(100 1000)
CALCULATORS=("ref" "line" "batch")

i=0
    for calc in "${CALCULATORS[@]}"; do
        for run in `seq 3`; do
            (
                for iter in "${ITERS[@]}"; do
                    for shape in "${SHAPES[@]}"; do
                        ./mandelbrot -s $shape -i $iter -c $calc --batch > "tmp_${calc}_${run}_${iter}_${shape}"
                    done
                done
            ) &
            p=$!
            echo $p
            pids[${i}]=$p
            echo "pids" ${pids[*]}
            i=$((i+1)) 
        done
    done

# wait for all pids
for pid in ${pids[*]}; do
    echo "wait for $pid"
    wait $pid
done



 (
    echo "CALCULATOR;BASE;WIDTH;HEIGHT;ITERS;TIME"
    for calc in "${CALCULATORS[@]}"; do
        for run in `seq 3`; do
            for iter in "${ITERS[@]}"; do
                for shape in "${SHAPES[@]}"; do
                    cat "tmp_${calc}_${run}_${iter}_${shape}"
                done
            done &
        done
    done
 ) | tee ../datalog.csv
