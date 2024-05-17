#!/usr/bin/python
import numpy as np
import matplotlib.pyplot as plt
import csv
from itertools import product
import argparse




def plot_data(filename = "datalog.csv", show = False, save = None):
    with open(filename) as inputFile:
        inputReader = csv.DictReader(inputFile, delimiter=";")

        data_reord = list(inputReader)

        keys = ["CALCULATOR", "BASE", "WIDTH", "HEIGHT", "ITERS", "TIME"]

        data = {k: np.array([x[k] for x in data_reord],
                            dtype="str" if k == "CALCULATOR" else "i") for k in keys}

    plt.figure(figsize=(12, 12))    

    base_sizes = np.sort(np.unique(data["BASE"]))
    calculators = np.unique(data["CALCULATOR"])
    iters = np.unique(data["ITERS"])

    calc_labels = [x.replace("MandelCalculator", "") for x in calculators]

    for i, (iter, b) in enumerate(product(iters, base_sizes)):
        ax = plt.subplot(2, 4, 1+i)
        ax.set_title(f"Grid: {3*b}x{2*b} Iters: {iter}")
        ax.boxplot(
            [   
                data["TIME"][(data["CALCULATOR"] == c) & 
                            (data["BASE"] == b) &
                            (data["ITERS"] == iter)]
                for c in calculators
            ]
        )

        ax.set(
            xticks = np.arange(len(calculators)) + 1,
            xticklabels = list(calc_labels),
            ylim = (0, None),
            ylabel = "Execution time [ms]"
        )

    plt.tight_layout()
    if save:
        plt.savefig(save)
        print(f"#saving to {save}")
    if show:
        plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare two npz files")
    parser.add_argument("filename", type=str, default="datalog.csv")
    parser.add_argument("--save", type=str, default=None)
    parser.add_argument("--show", action='store_const',
                    const=True, default=False,)


    args = parser.parse_args()

    print(vars(args))

    if not plot_data(**vars(args)):
        exit(1)