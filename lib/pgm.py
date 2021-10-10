import numpy as np
import matplotlib.pyplot as plt

def read(name):
    with open(name) as f:
        lines = f.readlines()

    # Ignores commented lines
    for line in list(lines):
        if line[0] == '#':
            lines.remove(line)

    # Makes sure it is ASCII format (P2)
    assert lines[0].strip() == 'P2' 

    # Converts data to a list of integers
    data = []
    for line in lines[1:]:
        data.extend([int(token) for token in line.split()])
    
    width = data[0]
    height = data[1]
    maxval = data[2]
    pixels = data[3:]
    
    return (np.array(pixels), (height, width), maxval)

def show(pgm, cmap='gray'):
    image, dims, vmax = pgm
    plt.imshow(np.reshape(image, dims), cmap=cmap, vmin=0, vmax=vmax)
    
def load_and_show(path):
    show(read(path))