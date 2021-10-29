import numpy as np

I_MIN = 0
I_MAX = 255

def apply_nd(fn, ndarr, *args):
    ndarr = np.array(ndarr)
    arr = fn(ndarr.flatten(), *args)
    return np.array(arr).reshape(ndarr.shape).astype(int).tolist()

def apply_minmax(arr, i_min=I_MIN, i_max=I_MAX):
    r_min = min(arr)
    r_max = max(arr)
    r_range = r_max - r_min
    i_range = i_max - i_min
    
    if r_range == 0:
        return np.zeros(len(arr)).tolist()
    
    for i, r in enumerate(arr):   
        arr[i] = int(round((r - r_min) / r_range * i_range + i_min))
        
    return arr
    
def apply_threshold(arr, tmin=I_MIN, tmax=I_MAX):
    for i, r in enumerate(arr):   
        if r < tmin:
            arr[i] = tmin
        elif r > tmax:
            arr[i] = tmax
    
    return arr

def apply_threshold_nd(ndarr, i_min=I_MIN, i_max=I_MAX):
    return apply_nd(apply_threshold, ndarr, i_min, i_max)

def apply_minmax_nd(ndarr, i_min=I_MIN, i_max=I_MAX):
    return apply_nd(apply_minmax, ndarr, i_min, i_max)