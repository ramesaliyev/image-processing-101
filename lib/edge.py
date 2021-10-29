import math
import numpy as np

from .utils import array2d, arrcopy
from .norm import apply_minmax_nd
from .smooth import create_gaussian_kernel
from .conv import convolve2d, pad_zero, apply_padding

def filter_edges(image, kernel_x, kernel_y, findDirections=False):
    im_width = len(image[0])
    im_height = len(image)
    kr_size = len(kernel_x)
    kr_padding = kr_size - 1
    out_width = im_width - kr_padding
    out_height = im_height - kr_padding   
    output = array2d(out_width, out_height)
    directions = None
    
    if findDirections:
        directions = array2d(out_width, out_height)
    
    for r in range(out_height):
        for c in range(out_width):
            output_x = 0
            output_y = 0
            for i in range(kr_size):
                for j in range(kr_size):
                    output_x += image[r+i][c+j] * kernel_x[i][j]
                    output_y += image[r+i][c+j] * kernel_y[i][j]
                    
            output[r][c] = math.sqrt(output_x**2 + output_y**2)
            
            if findDirections:
                dir_angle = math.degrees(math.atan2(output_y, output_x))
                dir_region = math.floor(((dir_angle + 360 + 22.5) % 180) / 45)
                directions[r][c] = dir_region
    
    if findDirections:
        return (output, directions)
    
    return output

def sobel(image, findDirections=False):
    sobel_kernel_x = [
        [-1, 0, 1],
        [-2, 0, 2],
        [-1, 0, 1]
    ]

    sobel_kernel_y = [
        [1, 2, 1],
        [0, 0, 0],
        [-1, -2, -1]
    ]
    
    return filter_edges(image, sobel_kernel_x, sobel_kernel_y, findDirections)

def prewitt(image, findDirections=False):
    prewitt_kernel_x = [
        [-1, 0, 1],
        [-1, 0, 1],
        [-1, 0, 1]
    ]

    prewitt_kernel_y = [
        [1, 1, 1],
        [0, 0, 0],
        [-1, -1, -1]
    ]
    
    return filter_edges(image, prewitt_kernel_x, prewitt_kernel_y, findDirections)

def canny(image, gauss_sigma, ht_low_ratio, ht_high_ratio, apply_nms=True, apply_ht=True):
    # 1) apply gaussian filter
    gauss_kernel_size = 5
    gauss_kernel = create_gaussian_kernel(gauss_kernel_size, gauss_sigma)
    image = convolve2d(image, gauss_kernel);
    
    # 2) find intensity gradient (aka edges with sobel)
    (image, directions) = sobel(image, findDirections=True)
    
    # Preparation
    pad_zero(image)
    pad_zero(directions)
    (image_h, image_w) = (len(image), len(image[0]))
    (range_y, range_x) = (image_h - 1, image_w - 1)
    
    # 3) apply non-maximum supression
    if apply_nms:
        image_next = array2d(image_w, image_h)
        
        for r in range(1, range_y):
            for c in range(1, range_x):
                pixel = image[r][c]
                dir_region = directions[r][c]
                neighboor_1 = 0
                neighboor_2 = 0

                if dir_region == 0:
                    neighboor_1 = image[r][c-1]
                    neighboor_2 = image[r][c+1]
                elif dir_region == 1:
                    neighboor_1 = image[r+1][c-1]
                    neighboor_2 = image[r-1][c+1]
                elif dir_region == 2:
                    neighboor_1 = image[r-1][c]
                    neighboor_2 = image[r+1][c]
                elif dir_region == 3:
                    neighboor_1 = image[r-1][c-1]
                    neighboor_2 = image[r+1][c+1]

                if pixel >= neighboor_1 and pixel >= neighboor_2:
                    image_next[r][c] = pixel
                else:
                    image_next[r][c] = 0
        
        image = image_next
        
    # 4) apply hysteresis-threshold
    if apply_ht:
        image_next = array2d(image_w, image_h)
        
        i_max = max(map(max, image))
        th = i_max * ht_high_ratio
        tl = i_max * ht_low_ratio
        
        for r in range(1, range_y):
            for c in range(1, range_x):
                pixel = image[r][c]
                
                if pixel >= th:
                    image_next[r][c] = 255
                elif pixel < tl:
                    image_next[r][c] = 0
                else:
                    neighboors = [
                        image[r-1][c-1],
                        image[r-1][c],
                        image[r-1][c+1],
                        image[r][c-1],
                        image[r][c+1],
                        image[r+1][c-1],
                        image[r+1][c],
                        image[r+1][c+1]
                    ]
                    
                    strong_neighboors = [
                        edge for edge in neighboors if edge >= th
                    ]
                    
                    if len(strong_neighboors) > 0:
                        image_next[r][c] = 255
                    else:
                        image_next[r][c] = 0
        
        image = image_next

    return image