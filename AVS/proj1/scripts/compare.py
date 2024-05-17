#!/usr/bin/python3

import numpy as np
import argparse


def main(file1=None, file2=None):

    fail = "[\033[91mfail\033[0m]"
    ok = "[\033[92mok\033[0m]"


    try:
        a1 = np.load(file1)["d"]
    except Exception as e:
        print(f"{fail} Error during loading {file1}: {e}")
        return False

    try:
        a2 = np.load(file2)["d"]
    except Exception as e:
        print(f"{fail} Error during loading {file2}: {e}")
        return False

    if(a1.shape != a2.shape):
        print(f"{fail} Sizes don't match ({a1.shape} vs {a2.shape})")
        return False

    diff = np.abs(a1 - a2)

    bool_a1 = a1 == a1.max()
    bool_a2 = a2 == a2.max()

    size = np.multiply.reduce(a1.shape)

    invalid = bool_a1 != bool_a2
    close = (invalid.sum() / size)

    if((diff <= 1).all()):
        print(f"{ok} Results are same")

    elif close < 0.001:
        print(f"{ok} Results are very close (eps = {close:.3%} )")
        return True
    else:
        cnt = np.sum(diff > 1)

        print(f"{fail} Results differs in {cnt} values")

        pos = (np.where(a1 != a2))

        for p in zip(pos[0], pos[1]):
            print(f"  - {p} {a1[p]} {a2[p]}")


        return False

    
    return True


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare two npz files")
    parser.add_argument("file1", type=str)
    parser.add_argument("file2", type=str)


    args = parser.parse_args()

    if not main(**vars(args)):
        exit(1)