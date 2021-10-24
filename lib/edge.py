import math
from .utils import array2d

def filter_edges(image, kernel_x, kernel_y):
    im_width = len(image[0])
    im_height = len(image)
    kr_size = len(kernel_x)
    kr_padding = kr_size - 1
    out_width = im_width - kr_padding
    out_height = im_height - kr_padding   
    output = array2d(out_width, out_height)
    
    for r in range(out_height):
        for c in range(out_width):
            output_x = 0
            output_y = 0
            for i in range(kr_size):
                for j in range(kr_size):
                    output_x += image[r+i][c+j] * kernel_x[i][j]
                    output_y += image[r+i][c+j] * kernel_y[i][j]
                    
            output[r][c] = math.sqrt(output_x**2 + output_y**2)
    
    return output

def prewitt(image):
    prewitt_kernel_x = [
        [-1, 0, 1],
        [-1, 0, 1],
        [-1, 0, 1]
    ]

    prewitt_kernel_y = [
        [-1, -1, -1],
        [0, 0, 0],
        [1, 1, 1]
    ]
    
    return filter_edges(image, prewitt_kernel_x, prewitt_kernel_y)

def sobel(image):
    sobel_kernel_x = [
        [-1, 0, 1],
        [-2, 0, 2],
        [-1, 0, 1]
    ]

    sobel_kernel_y = [
        [-1, -2, -1],
        [0, 0, 0],
        [1, 2, 1]
    ]
    
    return filter_edges(image, sobel_kernel_x, sobel_kernel_y)