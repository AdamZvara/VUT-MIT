import time
import os
import multiprocessing

# cores = multiprocessing.cpu_count()
cores = 16
ns = [pow(2, x) for x in range(1, cores)]

def to_int(list_of_string):
    list_of_string.remove("")
    return [int(x) for x in list_of_string]

def run_eval():
    with open("tmp", 'r') as file:
        file_content = file.read()

    split_lines = file_content.split("\n")

    input_seq = to_int(split_lines[0].split(" "))
    sorted_seq = to_int(split_lines[1:])

    input_seq.sort()
    if not input_seq == sorted_seq:
        print("Not sorted sequences :(")


for n in ns:
    tic = time.perf_counter()
    os.system(f"./run.sh {n} > tmp")
    toc = time.perf_counter()
    run_eval()
    print(f"Run with n={n} in {toc - tic:0.4f} seconds")