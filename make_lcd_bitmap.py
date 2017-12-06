#! /usr/bin/env python3
from PIL import Image
import sys

#USE PNG files as input.  In GIMP set palette to grayscale, and only use black or white to draw the image.

filename = sys.argv[1]
#filename = "cross.png"

if not filename.lower().endswith(".png"):
    print ("Please specify a PNG file as input.")

im = Image.open(filename, 'r')
pix_val = list(im.getdata())  #pix_val is the list that contains all
width, height = im.size;

if (height <= 0):
    print ("Error: input file has zero height.")
    exit()

if (width <= 0):
    print ("Error: input file has zero width.")
    exit()

print ("Source image width %u pixels, height %u pixels." % (width, height))
if (height % 8 != 0):
    print ("WARNING: Image height is not a multiple of 8.  The created array will cover an area larger than the image.")
    extra_lines = (8 - height % 8) * width
    extra_pels = [255] * extra_lines
    pix_val.extend(extra_pels)
    height += extra_lines
    print ("Adjusting output image height to %u pixels." % height)

output = []

#white is 255, black is 0
for line in range(0, height, 8):
    for col in range(width):
        val = 0;
        for pos in range (8):
            if pix_val[(line+pos) * width + col] < 128: #ie. if it should be considered black
                val += 1 << pos
        output.append(val)

outstr = ", ".join(["0x%02x" % x for x in output])
print ("uint8_t %s[%u] = {%u,%u,%s};" % (filename.split(".")[0], len(output) + 2, width, height, outstr))
print("0x%02x, " % val, end="")
        
#the pixel values which can be printed to see those values
