#!/usr/bin/env python3
"""Preview RGB565 raw file"""

from PIL import Image
import struct
import sys

def preview_raw(filename, width=320, height=240):
    """Load and display RGB565 raw file"""
    with open(filename, 'rb') as f:
        data = f.read()
    
    img = Image.new('RGB', (width, height))
    pixels = img.load()
    
    idx = 0
    for y in range(height):
        for x in range(width):
            # Read RGB565 little-endian
            rgb565 = struct.unpack('<H', data[idx:idx+2])[0]
            
            # Extract RGB components
            r = ((rgb565 >> 11) & 0x1F) << 3
            g = ((rgb565 >> 5) & 0x3F) << 2
            b = (rgb565 & 0x1F) << 3
            
            pixels[x, y] = (r, g, b)
            idx += 2
    
    img.show()
    print(f"Displayed {filename}: {width}x{height}")

if __name__ == '__main__':
    preview_raw('data/hpt_logo.raw')
