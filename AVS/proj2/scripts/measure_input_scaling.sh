#!/bin/bash

SCRIPT_ROOT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

NUM_THREADS=(1 2 4 8 16 32)
INPUT_FILE_NAMES=(010 020 040 080 160 320 642)

GRID_DIMS=64
SOLVERS=(loop tree)

INPUT_FILES_ROOT=$SCRIPT_ROOT_PATH"/../data/benchmark/"
OUTPUT_FILE="input_scaling_out.csv"

echo "BUILDER_NAME;INPUT_FILE;OUTPUT_FILE;GRID_SIZE;ISO_LEVEL;FIELD_ELEMENTS;NUM_THREADS;ELAPSED_TIME;TOTAL_TRIANGLES;OUTPUT_SIZE" > $OUTPUT_FILE


for((i=0; i<${#SOLVERS[@]}; i++)); do
    for ((j=0; j<${#NUM_THREADS[@]}; j++)); do
        for ((k=0; k<${#INPUT_FILE_NAMES[@]}; k++)); do
            input_file=$INPUT_FILES_ROOT${INPUT_FILE_NAMES[k]}".pts"
            ./PMC $input_file -l 0.15 -g $GRID_DIMS -b ${SOLVERS[i]} -t ${NUM_THREADS[j]} --batch >> $OUTPUT_FILE
        done
    done
done
