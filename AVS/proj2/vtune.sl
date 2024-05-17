#!/bin/bash
#SBATCH -p qcpu_exp
#SBATCH -A DD-23-135
#SBATCH -n 1 
#SBATCH --comment "use:vtune=2022.2.0"
#SBATCH -t 0:10:00
#SBATCH --mail-type END
#SBATCH -J AVS-vtune

cd $SLURM_SUBMIT_DIR

ml intel-compilers/2022.1.0 CMake/3.23.1-GCCcore-11.3.0 VTune/2022.2.0-intel-2021b

[ -d build_vtune ] && rm -rf build_vtune
[ -d build_vtune ] || mkdir build_vtune
cd build_vtune

CC=icc CXX=icpc cmake ..
make

for threads in 18 36; do
    for builder in "ref" "loop" "tree"; do
        rm -rf vtune-${builder}-${threads}
        vtune -collect threading -r vtune-${builder}-${threads} -app-working-dir . -- ./PMC --builder ${builder} -t ${threads} --grid 128 ../data/bun_zipper_res3.pts
    done
done
