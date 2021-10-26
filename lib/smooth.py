import math
import numpy as np
from .utils import array2d

def create_average_kernel(size):
    return array2d(size, size, 1/(size*size))

def calc_gaussian_kernel_cell(sigma, x, y):
    return (1/(2 * math.pi * sigma**2)) * math.exp(-((x**2 + y**2)/(2 * sigma**2)))

def create_gaussian_kernel(size, sigma):
    kernel = [0] * (size * size)

    for y in range(size):
        for x in range(size):
            margin = (size - 1) / 2
            kernel[y * size + x] = calc_gaussian_kernel_cell(sigma, x - margin, y - margin)

    kernel = np.array(kernel)
    kernel = kernel * (1/min(kernel))
    kernel = kernel * (1/sum(kernel))
    kernel = kernel.reshape((size, size)).tolist()
    
    return kernel

def apply_median_filter(image, kernel_size):
    im_width = len(image[0])
    im_height = len(image)
    kr_padding = kernel_size - 1
    out_width = im_width - kr_padding
    out_height = im_height - kr_padding
    output = array2d(out_width, out_height)
    image = np.array(image)
    median = int((kernel_size**2 - 1) / 2)
    
    for r in range(out_height):
        for c in range(out_width):
            kernel = image[r:r+kernel_size, c:c+kernel_size]
            kernel = np.sort(kernel.flatten())
            output[r][c] = kernel[median]
    
    return output