#!/bin/bash
#SBATCH -p qcpu_exp
#SBATCH -A DD-23-135
#SBATCH -n 1 
#SBATCH -t 0:45:00
#SBATCH --mail-type END
#SBATCH -J AVS-evaluate

cd $SLURM_SUBMIT_DIR

ml intel-compilers/2022.1.0 CMake/3.23.1-GCCcore-11.3.0 # pouze na Barbore

export OMP_PROC_BIND=close 
export OMP_PLACES=cores

[ -d build_evaluate ] && rm -rf build_evaluate
[ -d build_evaluate ] || mkdir build_evaluate

cd build_evaluate
rm tmp_*

CC=icc CXX=icpc cmake ..
make

ml matplotlib/3.5.2-foss-2022a
bash ../scripts/generate_data_and_plots.sh
