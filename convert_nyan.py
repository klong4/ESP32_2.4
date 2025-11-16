#!/usr/bin/env python3
"""Convert nyan cat frames to RGB565 format"""

from PIL import Image
import struct
import os

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565 format"""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

def convert_image(input_path, output_path):
    """Convert PNG to RGB565 raw format"""
    print(f"Converting {os.path.basename(input_path)}...")
    
    img = Image.open(input_path)
    img = img.convert('RGB')
    width, height = img.size
    
    with open(output_path, 'wb') as f:
        pixels = img.load()
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565(r, g, b)
                f.write(struct.pack('<H', rgb565))
    
    file_size = os.path.getsize(output_path)
    print(f"  Created {os.path.basename(output_path)} ({file_size} bytes)")

# Convert all 12 frames
input_dir = "src/ncat/full frame"
output_dir = "data"

for i in range(12):
    input_file = os.path.join(input_dir, f"frame_{i:02d}_delay-0.1s.png")
    output_file = os.path.join(output_dir, f"nyan_{i}.raw")
    
    if os.path.exists(input_file):
        convert_image(input_file, output_file)
    else:
        print(f"Warning: {input_file} not found")

print(f"\n✓ Conversion complete! Copy .raw files to SD card.")
