import numpy as np
import matplotlib.pyplot as plt

def pgm_to_image(pgm):
    (pixels, dims, *_) = pgm[:]
    return np.array(pixels).reshape(dims).tolist()

def show_image(image):
    vmin = min(map(min, image))
    vmax = max(map(max, image))
    
    plt.imshow(image, cmap='gray', vmin=vmin, vmax=vmax)