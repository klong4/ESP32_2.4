#!/usr/bin/env python3
"""
Convert PNG images to RGB565 raw format with color inversion
Also creates dark theme background
"""
from PIL import Image
import struct
import os

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565"""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

def invert_rgb565(color):
    """Invert RGB565 color"""
    return (~color) & 0xFFFF

def convert_image_to_rgb565(input_path, output_path, invert=False, target_size=None):
    """Convert image to RGB565 raw format"""
    img = Image.open(input_path).convert('RGB')
    
    if target_size:
        img = img.resize(target_size, Image.Resampling.LANCZOS)
    
    width, height = img.size
    pixels = img.load()
    
    with open(output_path, 'wb') as f:
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565(r, g, b)
                
                if invert:
                    rgb565 = invert_rgb565(rgb565)
                
                # Write as big-endian (network byte order)
                f.write(struct.pack('>H', rgb565))
    
    print(f"Converted {input_path} -> {output_path} ({width}x{height})")

def create_dark_background(output_path, width=320, height=240):
    """Create dark blue starfield background"""
    img = Image.new('RGB', (width, height), (10, 20, 40))  # Dark blue
    pixels = img.load()
    
    # Add stars
    import random
    random.seed(42)
    for _ in range(30):
        x = random.randint(0, width-1)
        y = random.randint(0, height-1)
        brightness = random.randint(150, 255)
        pixels[x, y] = (brightness, brightness, brightness)
    
    with open(output_path, 'wb') as f:
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565(r, g, b)
                rgb565 = invert_rgb565(rgb565)  # Invert for negative format
                f.write(struct.pack('>H', rgb565))
    
    print(f"Created dark background -> {output_path} ({width}x{height})")

# Main conversion
if __name__ == '__main__':
    input_dir = 'src/ncat/full frame'
    output_dir = 'data'
    
    os.makedirs(output_dir, exist_ok=True)
    
    # Create dark background for rolodex
    create_dark_background(f'{output_dir}/rolodex_bg.raw', 320, 240)
    
    # Convert nyan cat frames (assume they're 320x240 or will be resized)
    frame_files = sorted([f for f in os.listdir(input_dir) if f.endswith('.png')])
    
    for i, frame_file in enumerate(frame_files):
        input_path = os.path.join(input_dir, frame_file)
        output_path = f'{output_dir}/nyan_{i}.raw'
        
        # Nyan cat frames - resize to fit screen and invert
        convert_image_to_rgb565(input_path, output_path, invert=True, target_size=(120, 84))
    
    print(f"\nConversion complete! {len(frame_files)} frames processed.")
    print("Copy the 'data' folder contents to your SD card root.")
