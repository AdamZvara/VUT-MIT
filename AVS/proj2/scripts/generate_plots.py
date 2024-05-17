import argparse
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import csv

parser = argparse.ArgumentParser(description='Generate scaling plots from measured data.')
parser.add_argument('input', type=str, help='an input CSV file with measurement data')
parser.add_argument('output', type=str, help='an output plot filename')
parser.add_argument('plot_type', type=str, choices=['input_strong', 'input_weak', 'grid_scaling'])

args = parser.parse_args()

with open(args.input, newline='') as inputFile:
    inputReader = csv.DictReader(inputFile, delimiter=';', quotechar="\"")

    if args.plot_type == 'input_strong':
        tables = {}
        builderNames = set()
        threadCnts = set()
        fieldElemsCnts = set()
        for row in inputReader:
            builderNames.add(row['BUILDER_NAME'])
            threadCnts.add(int(row['NUM_THREADS']))
            fieldElemsCnts.add(int(row['FIELD_ELEMENTS']))

            tables[(row['BUILDER_NAME'], int(row['NUM_THREADS']), int(row['FIELD_ELEMENTS']))] = float(row['ELAPSED_TIME'])

        fig, ax = plt.subplots(nrows=1, ncols=2, sharey=True, figsize=(10, 5))
        for pltIdx in range(0, len(builderNames)):
            builderName = list(sorted(builderNames))[pltIdx]
            for elems in sorted(fieldElemsCnts):
                ax[pltIdx].plot(sorted(threadCnts), [tables[(builderName, t, elems)] for t in sorted(threadCnts)],
                                   label='{0}'.format(elems), marker='o')
            ax[pltIdx].set(xlabel='Threads', ylabel='Time [ms]', title='Strong scaling: {0}'.format(builderName))
            ax[pltIdx].set_xscale('log', base=2)
            ax[pltIdx].set_yscale('log', base=2)
            ax[pltIdx].grid()

        handles, labels = ax[0].get_legend_handles_labels()
        fig.legend(handles, labels, title='Input\nsize')

        fig.savefig(args.output)

    elif args.plot_type == 'input_weak':
        tables = {}
        builderNames = set()
        threadCnts = set()
        fieldElemsCnts = set()
        for row in inputReader:
            builderNames.add(row['BUILDER_NAME'])
            threadCnts.add(int(row['NUM_THREADS']))
            fieldElemsCnts.add(int(row['FIELD_ELEMENTS']))

            tables[(row['BUILDER_NAME'], int(row['NUM_THREADS']), int(row['FIELD_ELEMENTS']))] = float(row['ELAPSED_TIME'])

        fig, ax = plt.subplots(nrows=1, ncols=2, sharey=True, figsize=(10, 5))
        for pltIdx in range(0, len(sorted(builderNames))):
            builderName = list(sorted(builderNames))[pltIdx]
            fieldElemsCntsList = list(sorted(fieldElemsCnts))
            threadCntsList = list(sorted(threadCnts))

            for elemsIdx in range(0, len(fieldElemsCntsList)):
                localElemsCount = fieldElemsCntsList[elemsIdx]
                if min(len(fieldElemsCnts), len(threadCnts)) <= elemsIdx:
                    break

                threads = threadCntsList[0:(min(len(fieldElemsCnts), len(threadCnts)) - elemsIdx)]
                times = [tables[(builderName, threadCntsList[n - elemsIdx], fieldElemsCntsList[n])]
                         for n in range(elemsIdx, min(len(fieldElemsCnts), len(threadCnts)))]

                ax[pltIdx].plot(threads, times, label='{0}'.format(localElemsCount), marker='o')

            ax[pltIdx].set(xlabel='Threads', ylabel='Time [ms]', title='Weak scaling: {0}'.format(builderName))
            ax[pltIdx].set_xscale('log', base=2)
            ax[pltIdx].set_yscale('log', base=2)
            ax[pltIdx].grid()

        handles, labels = ax[0].get_legend_handles_labels()
        fig.legend(handles, labels, title='Input\nsize\nper thread')

        fig.savefig(args.output)

    elif args.plot_type == 'grid_scaling':
        tables = {}
        builderNames = set()
        gridSizes = set()
        for row in inputReader:
            builderNames.add(row['BUILDER_NAME'])
            gridSizes.add(int(row['GRID_SIZE']))
            tables[(row['BUILDER_NAME'], int(row['GRID_SIZE']))] = float(row['ELAPSED_TIME'])

        fig, ax = plt.subplots()
        for name in sorted(builderNames):
            ax.plot(list(map(lambda x: x**3, sorted(gridSizes))), [tables[(name, n)] for n in sorted(gridSizes)],
                    label='{0}'.format(name), marker='o')

        ax.set_xscale('log', base=2)
        ax.set_yscale('log', base=8)
        ax.set(xlabel='Grid elements (-g value cubed)', ylabel='Time [ms]', title='Grid size scaling')
        ax.grid()

        handles, labels = ax.get_legend_handles_labels()
        fig.legend(handles, labels)

        fig.savefig(args.output)
