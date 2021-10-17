def array2d(col, row, val=0):
    return [[val for x in range(col)] for y in range(row)]

def flatten2d(arr):
    return [item for sublist in arr for item in sublist]