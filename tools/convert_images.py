#!/usr/bin/env python3
"""
Convert GIF frames and background to RGB565 raw format for ESP32-S3 SD card
Images will be scaled to fit 320x240 display in landscape mode
"""

from PIL import Image
import os
import struct

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565 format"""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

def convert_image_to_rgb565(input_path, output_path, target_width, target_height):
    """Convert image to RGB565 raw format"""
    print(f"Converting {input_path}...")
    
    # Open and convert image
    img = Image.open(input_path)
    
    # Convert to RGB if needed
    if img.mode != 'RGB':
        img = img.convert('RGB')
    
    # Resize to target dimensions
    img = img.resize((target_width, target_height), Image.Resampling.NEAREST)
    
    # Convert to RGB565 and write
    with open(output_path, 'wb') as f:
        pixels = img.load()
        for y in range(target_height):
            for x in range(target_width):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565(r, g, b)
                # Write as big-endian (MSB first)
                f.write(struct.pack('>H', rgb565))
    
    file_size = os.path.getsize(output_path)
    print(f"  -> {output_path} ({file_size} bytes)")
    return file_size

def main():
    # Paths
    source_dir = "../src/ncat"
    output_dir = "../sd_card"
    
    # Create output directory
    os.makedirs(output_dir, exist_ok=True)
    
    print("=" * 60)
    print("Converting Nyan Cat images to RGB565 format")
    print("=" * 60)
    
    # Convert Nyan Cat frames (scale to reasonable size)
    # Original frames appear to be small, scale them up
    nyan_width = 120
    nyan_height = 84
    
    total_size = 0
    
    for i in range(6):
        input_file = os.path.join(source_dir, f"frame_{i}.gif")
        output_file = os.path.join(output_dir, f"nyan_{i}.raw")
        
        if os.path.exists(input_file):
            size = convert_image_to_rgb565(input_file, output_file, nyan_width, nyan_height)
            total_size += size
        else:
            print(f"Warning: {input_file} not found!")
    
    # Convert background (full screen 320x240 landscape)
    bg_input = os.path.join(source_dir, "nyan_bg.jpg")
    bg_output = os.path.join(output_dir, "nyan_bg.raw")
    
    if os.path.exists(bg_input):
        size = convert_image_to_rgb565(bg_input, bg_output, 320, 240)
        total_size += size
    else:
        print(f"Warning: {bg_input} not found!")
    
    # Convert rainbow trail (tile it horizontally)
    rainbow_input = os.path.join(source_dir, "rainbow.png")
    rainbow_output = os.path.join(output_dir, "rainbow.raw")
    
    if os.path.exists(rainbow_input):
        size = convert_image_to_rgb565(rainbow_input, rainbow_output, 320, 132)
        total_size += size
    else:
        print(f"Warning: {rainbow_input} not found!")
    
    print("=" * 60)
    print(f"Conversion complete! Total size: {total_size} bytes ({total_size/1024:.1f} KB)")
    print(f"Files saved to: {output_dir}")
    print()
    print("Next steps:")
    print("1. Copy all .raw files from sd_card/ to the root of your SD card")
    print("2. Insert SD card into the display module")
    print("3. Upload the firmware to ESP32-S3")
    print("=" * 60)

if __name__ == "__main__":
    main()
