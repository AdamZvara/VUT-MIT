#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors
import argparse


def plot_visualize(filename="datalog.csv", show=False, save=None):
    res = np.load(filename)["d"]
    width, height = res.shape

    dpi = 72
    xmin, xmax = -2.0, 1.0
    ymin, ymax = -1.5, 1.5

    fig = plt.figure(figsize=(10, 10), dpi=dpi)
    ax = fig.add_axes([0.0, 0.0, 1.0, 1.0], frameon=False, aspect=1)

    light = colors.LightSource(azdeg=315, altdeg=10)
    plt.imshow(light.shade(res, cmap=plt.cm.hot, vert_exag=1.5,
                           norm=colors.PowerNorm(0.2), blend_mode='hsv'),
               extent=[xmin, xmax, ymin, ymax], interpolation="bicubic"
               )

    ax.set(xlabel="Real", ylabel="Imag")

    if save:
        plt.savefig(save)
        print(f"#saving to {save}")
    if show:
        plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Visualize mandelbrot set")
    parser.add_argument("filename", type=str, default="res.npz")
    parser.add_argument("--save", type=str, default=None)
    parser.add_argument("--show", action='store_const',
                        const=True, default=False,)

    args = parser.parse_args()

    print(vars(args))

    if not plot_visualize(**vars(args)):
        exit(1)
# %%

# %%
