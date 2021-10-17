from .utils import array2d

def convolve2d(image, kernel):
    im_width = len(image[0])
    im_height = len(image)
    kr_size = len(kernel)
    kr_padding = kr_size - 1
    kr_offset = kr_padding / 2
    out_width = im_width - kr_padding
    out_height = im_height - kr_padding
    output = array2d(out_width, out_height)
    
    for r in range(out_height):
        for c in range(out_width):
            for i in range(kr_size):
                for j in range(kr_size):
                    output[r][c] += image[r+i][c+j] * kernel[i][j]
    
    return output

def pad_color(image, color):
    width = len(image[0])
        
    for line in image:
        line.insert(0, color)
        line.append(color)
    
    borders = [color] * (width + 2)
    image.insert(0, borders[:])
    image.append(borders)

def pad_zero(image):
    pad_color(image, 0)

def pad_mirror(image):
    width = len(image[0])
    height = len(image)
    
    for line in image:
        line.append(line[width-1])
        line.insert(0, line[0])
    
    top = image[0][:]
    bottom = image[height-1][:]
    image.insert(0, top)
    image.append(bottom)
    
def apply_padding(image, padding, n):
    for i in range(n):
        padding(image)

def create_average_kernel(size):
    return array2d(size, size, 1/(size*size))