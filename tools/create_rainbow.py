#!/usr/bin/env python3
"""
Create rainbow trail image based on Nyan Cat rainbow pattern
"""

from PIL import Image

def create_rainbow():
    # Rainbow colors (6 bands)
    colors = [
        (255, 0, 0),      # Red
        (255, 153, 0),    # Orange
        (255, 255, 0),    # Yellow
        (0, 255, 0),      # Green
        (102, 204, 255),  # Light Blue
        (0, 0, 255),      # Blue
    ]
    
    # Create image: 840x132 pixels (will be scaled to 320x132)
    # Each color band is 22 pixels tall
    # Pixelated pattern with blocks
    width = 840
    band_height = 22
    height = band_height * 6
    
    img = Image.new('RGB', (width, height), (0, 0, 0))
    pixels = img.load()
    
    # Draw each colored band with pixelated pattern
    for band_idx, color in enumerate(colors):
        y_start = band_idx * band_height
        
        # Create pixelated block pattern
        block_size = 7  # Size of blocks
        gap_size = 3    # Size of gaps between blocks
        pattern_width = block_size + gap_size
        
        for y in range(band_height):
            for x in range(width):
                # Determine if we're in a block or gap
                block_x = (x // pattern_width) * pattern_width
                x_in_pattern = x % pattern_width
                
                # Offset pattern for each row to create staggered effect
                row_offset = (y // block_size) * (pattern_width // 2)
                x_offset = (x + row_offset) % width
                x_in_pattern = x_offset % pattern_width
                
                # Draw block or transparent area
                if x_in_pattern < block_size and y % band_height < band_height:
                    pixels[x, y_start + y] = color
                else:
                    # Transparent (will show background)
                    pixels[x, y_start + y] = (0, 0, 0)
    
    # Save image
    output_path = "../src/ncat/rainbow.png"
    img.save(output_path)
    print(f"Rainbow image created: {output_path}")
    print(f"Dimensions: {width}x{height}")
    
    return output_path

if __name__ == "__main__":
    create_rainbow()
