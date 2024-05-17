#!/bin/bash
#SBATCH -p qcpu_exp
#SBATCH -A DD-23-135
#SBATCH -n 1 
#SBATCH -t 0:5:00

cd $SLURM_SUBMIT_DIR

ml intel-compilers/2022.1.0 CMake/3.23.1-GCCcore-11.3.0

[ -d build_run ] && rm -rf build_run
[ -d build_run ] || mkdir build_run
cd build_run

CC=icc CXX=icpc cmake ..
make

SCRIPT_ROOT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

GRID_DIMS=(32 64)
NUM_THREADS=36
SOLVERS=(ref)

# INPUT_FILES=(bun_zipper_res1.pts bun_zipper_res2.pts bun_zipper_res3.pts bun_zipper_res4.pts dragon_vrip_res1.pts dragon_vrip_res2.pts dragon_vrip_res3.pts dragon_vrip_res4.pts)
INPUT_FILES=(bun_zipper_res2.pts bun_zipper_res3.pts bun_zipper_res4.pts dragon_vrip_res3.pts dragon_vrip_res4.pts)

echo "BUILDER_NAME;INPUT_FILE;OUTPUT_FILE;GRID_SIZE;ISO_LEVEL;FIELD_ELEMENTS;NUM_THREADS;ELAPSED_TIME;TOTAL_TRIANGLES;OUTPUT_SIZE"

for((i=0; i<${#SOLVERS[@]}; i++)); do
    for ((j=0; j<${#GRID_DIMS[@]}; j++)); do
        for((k=0; k<${#INPUT_FILES[@]}; k++)); do
            ./PMC "/home/xzvara01/proj/avs-proj02/data/${INPUT_FILES[k]}" -l 0.15 -g ${GRID_DIMS[j]} -b ${SOLVERS[i]} -t $NUM_THREADS >> ${SOLVERS[i]}.out
        done
    done
done