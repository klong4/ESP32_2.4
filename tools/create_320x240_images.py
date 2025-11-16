#!/usr/bin/env python3
"""
Convert images to 320x240 RGB565 raw format for ESP32-S3 screensaver.
Requires: pip install pillow
"""

from PIL import Image
import struct
import os

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565 format"""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

def convert_image_to_rgb565(input_path, output_path, width=320, height=240):
    """Convert an image to RGB565 raw format"""
    print(f"Converting {input_path}...")
    
    # Open and resize image to 320x240
    img = Image.open(input_path)
    img = img.convert('RGB')  # Ensure RGB format
    img = img.resize((width, height), Image.Resampling.LANCZOS)
    
    # Convert to RGB565 and save
    with open(output_path, 'wb') as f:
        pixels = img.load()
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565(r, g, b)
                # Write as little-endian 16-bit value
                f.write(struct.pack('<H', rgb565))
    
    file_size = os.path.getsize(output_path)
    expected_size = width * height * 2
    print(f"  Created {output_path} ({file_size} bytes, expected {expected_size})")
    
    if file_size != expected_size:
        print(f"  WARNING: File size mismatch!")

def main():
    """Convert nyan cat frames to 320x240 RGB565"""
    
    # Define paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.dirname(script_dir)
    input_dir = os.path.join(project_dir, "source_images")  # Put your PNG/JPG files here
    output_dir = os.path.join(project_dir, "data")
    
    print(f"Input directory: {input_dir}")
    print(f"Output directory: {output_dir}")
    
    if not os.path.exists(input_dir):
        os.makedirs(input_dir)
        print(f"\nCreated {input_dir}")
        print("Please place your source images (PNG/JPG) in this directory:")
        print("  - nyan_0.png")
        print("  - nyan_1.png")
        print("  - ... up to nyan_11.png")
        return
    
    # Convert all nyan frames
    converted_count = 0
    for i in range(12):
        # Try common image formats
        input_file = None
        for ext in ['.png', '.jpg', '.jpeg', '.bmp']:
            test_path = os.path.join(input_dir, f"nyan_{i}{ext}")
            if os.path.exists(test_path):
                input_file = test_path
                break
        
        if input_file:
            output_file = os.path.join(output_dir, f"nyan_{i}.raw")
            convert_image_to_rgb565(input_file, output_file)
            converted_count += 1
        else:
            print(f"Skipping nyan_{i} - source file not found")
    
    print(f"\n✓ Converted {converted_count} images to 320x240 RGB565 format")
    print(f"✓ Copy the .raw files from '{output_dir}' to your SD card")

if __name__ == "__main__":
    main()
