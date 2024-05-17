#!/bin/bash

SCRIPT_ROOT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

if [[ ! -f "grid_scaling_out.csv" ]]; then
    echo "Measuring grid size scaling"
    $SCRIPT_ROOT_PATH/measure_grid_scaling.sh
else
    echo "Grid size scaling results found"
fi

if [[ ! -f "input_scaling_out.csv" ]]; then
    echo "Measuring input size scaling"
    $SCRIPT_ROOT_PATH/measure_input_scaling.sh
else
    echo "Input size scaling results found"
fi

echo "Creating scaling plots"
python3 $SCRIPT_ROOT_PATH/generate_plots.py input_scaling_out.csv $SCRIPT_ROOT_PATH/../input_scaling_strong.png input_strong
python3 $SCRIPT_ROOT_PATH/generate_plots.py input_scaling_out.csv $SCRIPT_ROOT_PATH/../input_scaling_weak.png input_weak
python3 $SCRIPT_ROOT_PATH/generate_plots.py grid_scaling_out.csv $SCRIPT_ROOT_PATH/../grid_scaling.png grid_scaling
