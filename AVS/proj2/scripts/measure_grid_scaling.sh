#!/bin/bash

SCRIPT_ROOT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

GRID_DIMS=(8 16 32 64 128 256 512)
NUM_THREADS=36
SOLVERS=(loop tree)

INPUT_FILE=$SCRIPT_ROOT_PATH"/../data/bun_zipper_res3.pts"
OUTPUT_FILE="grid_scaling_out.csv"

echo "BUILDER_NAME;INPUT_FILE;OUTPUT_FILE;GRID_SIZE;ISO_LEVEL;FIELD_ELEMENTS;NUM_THREADS;ELAPSED_TIME;TOTAL_TRIANGLES;OUTPUT_SIZE" > $OUTPUT_FILE

for((i=0; i<${#SOLVERS[@]}; i++)); do
    for ((j=0; j<${#GRID_DIMS[@]}; j++)); do
        ./PMC $INPUT_FILE -l 0.15 -g ${GRID_DIMS[j]} -b ${SOLVERS[i]} -t $NUM_THREADS --batch >> $OUTPUT_FILE
    done
done
