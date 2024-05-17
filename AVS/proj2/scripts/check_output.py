import argparse
import numpy as np

parser = argparse.ArgumentParser(description='Compare outputs to the reference.')
parser.add_argument('ref_input', type=str, help='an output of the reference implementation')
parser.add_argument('test_input', type=str, help='an output of the implementation under the test')

args = parser.parse_args()

def readVerticesFromFile(filename):
    vertices = []
    with open(filename, 'r') as inputFile:
        for line in inputFile:
            if line.strip().startswith('v'):
                vertex = line.strip().split(' ')
                xyz = tuple(map(lambda x: float(x), vertex[1:4]))
                vertices.append(xyz)
    
    return np.array(vertices, dtype=[('x', 'f4'), ('y', 'f4'), ('z', 'f4')])


ref_vertices  = readVerticesFromFile(args.ref_input)
test_vertices = readVerticesFromFile(args.test_input)

print("Reference mesh vertices count: {}".format(ref_vertices.shape[0]))
print("Test mesh vertices count:      {}".format(test_vertices.shape[0]))

if ref_vertices.shape != test_vertices.shape:
    print("Number of generated vertices doesn't match!")
    exit(1)

ref_vertices = np.sort(ref_vertices)
test_vertices = np.sort(test_vertices)

ref_vertices  = ref_vertices.view(np.float32).reshape(ref_vertices.shape + (-1,))
test_vertices = test_vertices.view(np.float32).reshape(test_vertices.shape + (-1,))

max_distance = np.sqrt(np.max(np.sum(np.square(test_vertices - ref_vertices), axis=1)))

print("Maximum distance between vertices: {}".format(max_distance))

if max_distance > 0.05:
    exit(1)