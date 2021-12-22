# Libraries
# Matplotlib used to read jpg images and also show images on screen. 
import matplotlib.pyplot as plt
import matplotlib.image as img
import os
import math
import json


# Helper Functions
def readimage(path):
    return img.imread(path)

def showimage(image):
    plt.imshow(image)
    
def savejson(name, data):
    with open(name, 'w') as outfile:
        json.dump(data, outfile)
        
def loadjson(name):
    with open(name) as jsonfile:
        data = json.load(jsonfile)
    return data

def getfiles(dirpath):
    files = next(os.walk(dirpath), (None, None, []))[2]
    files = [file for file in files if file[0] != '.']
    files.sort()
    return files


# RGB to Hue Conversion Function
def RGB2Hue(r, g, b):
    r = r / 255
    g = g / 255
    b = b / 255
    
    maxc = max(r, g, b)
    minc = min(r, g, b)
    rang = maxc - minc
    hue = 0
    
    if rang == 0:
        return 0
    
    if maxc == r:
        hue = (g-b)/rang
    elif maxc == g:
        hue = 2.0 + (b-r)/rang
    else:
        hue = 4.0 + (r-g)/rang
    
    return (round(hue * 60) + 360) % 360


# Histogram Calculation Function
# Will calculate all of four histograms.
def histogram(image):
    R = [0] * 256
    G = [0] * 256
    B = [0] * 256
    H = [0] * 360
    
    unit = 1.0 / (len(image) * len(image[0]))
    
    for row in image:
        for pixel in row:
            r, g, b = pixel
            h = RGB2Hue(r, g, b)
            R[r] += unit
            G[g] += unit
            B[b] += unit
            H[h] += unit
        
    return (R, G, B, H)


# Distance Calculation Functions
# Euclidean distance calculation approach used.
def eucdist(vecA, vecB):
    pairs = list(zip(vecA, vecB))
    total = sum((a-b)**2 for a, b in pairs)
    return math.sqrt(total)

def histdist(histsA, histsB):
    histpairs = list(zip(histsA, histsB))
    histdists = list(map(lambda p: eucdist(*p), histpairs))
    return histdists

def imagedist(histsA, histsB):
    histdists = histdist(histsA, histsB)
    rgbdist = eucdist([0,0,0], histdists[:3]) / math.sqrt(3) # max distance
    huedist = histdists[-1]
    return (rgbdist, huedist)

def similarity(histsA, histsB):
    rgbdist, huedist = imagedist(histsA, histsB)
    return (1 - rgbdist, 1 - huedist)


# Train Function
# Will collect histograms of training images.
def train(root, folders, size):
    model = []
    
    for folder in folders:
        dirpath = os.path.join(root, folder)
        files = getfiles(dirpath)
        
        for file in files[:size]:
            abspath = os.path.join(dirpath, file)
            print(f"Training {abspath}") 
            
            image = readimage(abspath)
            hists = histogram(image)
            
            model.append({
                'path': abspath,
                'category': folder,
                'histograms': hists
            })
    
    return model
    
def loadmodel():
    return loadjson('model.json')


# Function to Retrieve Most Similar Images
# Function will take model, image and n as inputs.
# Will retrieve most similar n images of given image from model.
def retrieve(model, image, n):
    targethists = histogram(image)
    similars = []
    
    for source in model:
        sourcehists = source['histograms']
        closeness = similarity(targethists, sourcehists)
        
        similars.append({
            'rgb': closeness[0],
            'hue': closeness[1],
            'path': source['path'],
            'category': source['category']
        })
    
    rgb = sorted(similars, key=lambda x: x['rgb'], reverse=True)
    hue = sorted(similars, key=lambda x: x['hue'], reverse=True)
    
    return (rgb[:n], hue[:n])


# Function to Find Most Similar Images of Test Images
# This function will walk through all test images and find out most
# similar 5 images for each.
def test(model, root, folders, after):
    results = []
    
    for category in folders:
        dirpath = os.path.join(root, category)
        files = getfiles(dirpath)
        
        for file in files[after:]:
            abspath = os.path.join(dirpath, file)
            
            result = {
                'path': abspath,
                'category': category,
                'rgb':{'success':False, 'results':[]},
                'hue':{'success':False, 'results':[]}
            }
            
            image = readimage(abspath)
            rgb, hue = retrieve(model, image, 5)
            
            result['rgb']['success'] = any(match['category'] == category for match in rgb)
            result['hue']['success'] = any(match['category'] == category for match in hue)
            result['rgb']['results'] = rgb
            result['hue']['results'] = hue
            
            # print(f"Testing {abspath} {result['rgb']['success']} {result['hue']['success']}") 
            
            results.append(result)
            
    return results


# Analyzer Function
# Analyze results to find out success rates.
def analyze(results):
    success = {'rgb':0, 'hue':0}
    rates = {'rgb':0, 'hue':0}
    
    report = {
        'total':0,
        'success':success,
        'rates':rates,
        'categories':{}
    }
    cats = []
    
    for result in results:
        report['total'] += 1
        
        cat = result['category']
        if cat not in report['categories']:
            cats.append(cat)
            report['categories'][cat] = {
                'total':0,
                'success':{'rgb':0, 'hue':0},
                'rates':{'rgb':0, 'hue':0}
            }
        
        catobj = report['categories'][cat]
        rgbsuccess = int(result['rgb']['success'])
        huesuccess = int(result['hue']['success'])
        
        success['rgb'] += rgbsuccess
        success['hue'] += huesuccess
        catobj['total'] += 1
        catobj['success']['rgb'] += rgbsuccess
        catobj['success']['hue'] += huesuccess
    
    for cat in cats:
        obj = report['categories'][cat]
        obj['rates']['rgb'] = round(obj['success']['rgb'] / obj['total'], 2)
        obj['rates']['hue'] = round(obj['success']['hue'] / obj['total'], 2)
    
    rates['rgb'] = round(success['rgb'] / report['total'], 2)
    rates['hue'] = round(success['hue'] / report['total'], 2)
    
    return report


# Main Flow

# Configure Dataset
ds_root = '../images'
ds_folders = ['elephant', 'flamingo', 'kangaroo', 'leopards', 'octopus', 'seahorse']
ds_train_size = 20

# Train Model
# Generate (Train model on your first run.)
model = train(ds_root, ds_folders, ds_train_size)
savejson('model.json', model)
print("Training done, model saved.")
# or Load (For other runs, load trained model.)
# model = loadmodel()

# Find Results
# Generate (Generate results on your first run.) 
results = test(model, ds_root, ds_folders, ds_train_size)
savejson('results.json', results)
print("Testing done, results saved.")
# or Load (For other runs, use generated results.)
# results = loadjson('results.json')

# Generate Report
report = analyze(results)