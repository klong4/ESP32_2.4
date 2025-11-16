#!/usr/bin/env python3
"""Convert HPT logo to RGB565 format for boot screen"""

from PIL import Image, ImageDraw, ImageFont
import struct
import os

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565 format with byte swap and invert"""
    # First convert to RGB565
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    rgb565 = (r5 << 11) | (g6 << 5) | b5
    
    # Apply byte swap
    swapped = (rgb565 >> 8) | ((rgb565 & 0xFF) << 8)
    
    # Apply invert
    inverted = ~swapped & 0xFFFF
    
    return inverted

def convert_logo(input_path, output_path, bg_color=(0, 0, 0)):
    """Convert PNG with transparency to RGB565 raw format with full screen splash"""
    print(f"Converting {input_path}...")
    
    # Create full screen image (320x240)
    full_img = Image.new('RGB', (320, 240), bg_color)
    draw = ImageDraw.Draw(full_img)
    
    # Load and composite logo
    logo = Image.open(input_path)
    logo = logo.convert('RGBA')
    logo_width, logo_height = logo.size
    
    print(f"  Logo size: {logo_width}x{logo_height}")
    
    # Create temporary background for logo compositing
    logo_bg = Image.new('RGB', (logo_width, logo_height), bg_color)
    logo_bg.paste(logo, (0, 0), logo)
    
    # Center logo on screen (320x240)
    logo_x = (320 - logo_width) // 2
    logo_y = (240 - logo_height) // 2
    full_img.paste(logo_bg, (logo_x, logo_y))
    
    # Add "Touch to Start" text at bottom
    try:
        # Try to use a nice font
        font = ImageFont.truetype("arial.ttf", 20)
    except:
        # Fallback to default font
        font = ImageFont.load_default()
    
    text = "Touch to Start"
    # Get text bounding box for centering
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    
    text_x = (320 - text_width) // 2
    text_y = 200
    
    draw.text((text_x, text_y), text, fill=(255, 255, 255), font=font)
    
    # Convert to RGB565
    with open(output_path, 'wb') as f:
        pixels = full_img.load()
        for y in range(240):
            for x in range(320):
                r, g, b = pixels[x, y]
                rgb565 = rgb888_to_rgb565(r, g, b)
                f.write(struct.pack('<H', rgb565))
    
    file_size = os.path.getsize(output_path)
    print(f"  Created {output_path} ({file_size} bytes)")
    print(f"  Full screen: 320x240 with centered logo and text")

# Convert HPT logo with black background
convert_logo("src/ncat/HPT.png", "data/boot_splash.raw", bg_color=(0, 0, 0))

print("\n✓ Logo converted! Copy boot_splash.raw to SD card.")
