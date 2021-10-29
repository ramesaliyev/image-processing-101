import numpy as np

def flatten(ndarray):
    return np.array(ndarray).flatten().tolist()

def array2d(x, y, val=0):
    return np.full((y, x), val).tolist()

def arrcopy(ndarray):
    return np.array(ndarray).copy().tolist()