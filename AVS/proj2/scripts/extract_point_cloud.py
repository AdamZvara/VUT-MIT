import argparse

parser = argparse.ArgumentParser(description='Extract point cloud from *.ply files.')
parser.add_argument('input', type=str, help='an input file in *.ply format')
parser.add_argument('output', type=str, help='an output file')
parser.add_argument('--scale', type=float, help='output points position scaling', default=1.0)

args = parser.parse_args()

with open(args.input, 'r') as inputFile:
    if next(inputFile).strip() != 'ply':
        raise ValueError('Unknown input file format')

    vertexCount = 0
    for line in inputFile:
        if line.strip().startswith('element vertex'):
            _, _, vertexCount = line.split(' ')
            vertexCount = int(vertexCount)
        elif line.strip().startswith('end_header'):
            break

    print('Converting "{0}" to "{1}"'.format(args.input, args.output))
    print('Processing: {0} vertices... '.format(vertexCount), end='', flush=True)

    with open(args.output, 'w+') as outputFile:
        for _ in range(vertexCount):
            line = next(inputFile).strip().split(' ')
            xyz = tuple(map(lambda x: float(x)*args.scale, line[0:3]))
            outputFile.write('p {0:.6f} {1:.6f} {2:.6f}\n'.format(*xyz))

    print('done')
