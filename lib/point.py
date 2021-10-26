from .pgm import calculate_histogram

def negative(pixels, i_max=255):
    for i, r in enumerate(pixels):
        pixels[i] =  i_max - r
        
def brightness_increase(pixels, g, i_max=255):
    for i, r in enumerate(pixels):
        pixels[i] = r + g if r + g <= i_max else r

def brightness_decrease(pixels, g, i_max=255):
    for i, r in enumerate(pixels):
        pixels[i] = r - g if r - g >= 0 else r

def threshold(pixels, k=127):
    for i, r in enumerate(pixels):
        pixels[i] =  255 if r > k else 0
        
def contrast_stretch(pixels, i_min=0, i_max=255):
    r_min = min(pixels)
    r_max = max(pixels)
    
    for i, r in enumerate(pixels):
        pixels[i] = round((r - r_min) * (i_max - i_min) / (r_max - r_min) + i_min)
        
def contrast_stretch_tailcut(pixels, histogram, cutoff=0.05, i_min=0, i_max=255):
    histogram = [(r, c) for r, c in enumerate(histogram) if c > 0]
    histogram_len = len(histogram)
    cut_index = round(cutoff * histogram_len)
       
    r_min = histogram[cut_index][0]
    r_max = histogram[histogram_len - 1 - cut_index][0]
    
    for i, r in enumerate(pixels):
        pixels[i] = round((r - r_min) * (i_max - i_min) / (r_max - r_min) + i_min)

def contrast_stretch_percentage(pixels, intensity_histogram, cutoff_percent=0.05, i_min=0, i_max=255):
    max_intensity = max(intensity_histogram)
    cut_intensity = max_intensity * cutoff_percent
    len_intensity = len(intensity_histogram)
    
    r_min = 0
    r_max = 0
    
    for r, intensity in enumerate(intensity_histogram):
        if intensity >= cut_intensity:
            r_min = r
            break
            
    for r, intensity in enumerate(reversed(intensity_histogram)):
        if intensity >= cut_intensity:
            r_max = len_intensity - 1 - r
            break
    
    for i, r in enumerate(pixels):
        pixels[i] = round((r - r_min) * (i_max - i_min) / (r_max - r_min) + i_min)