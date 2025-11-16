from PIL import Image

# Create rainbow with pixelated pattern like Nyan Cat
width = 840
height = 132

img = Image.new('RGB', (width, height))
pixels = img.load()

# 6 rainbow colors
colors = [
    (255, 0, 0),        # Red
    (255, 153, 0),      # Orange  
    (255, 255, 0),      # Yellow
    (0, 255, 0),        # Green
    (102, 204, 255),    # Light Blue
    (0, 0, 255)         # Blue
]

band_height = 22
block_width = 128
gap_width = 20

for band in range(6):
    y_start = band * band_height
    color = colors[band]
    
    for y in range(band_height):
        # Create pixelated block pattern
        for x in range(width):
            # Three blocks per pattern (like the image)
            pattern_pos = x % (block_width * 3 + gap_width * 3)
            
            # Determine if in a block or gap
            if pattern_pos < block_width:  # First block
                pixels[x, y_start + y] = color
            elif pattern_pos < block_width + gap_width:  # First gap
                pixels[x, y_start + y] = (0, 0, 0)
            elif pattern_pos < block_width * 2 + gap_width:  # Second block
                pixels[x, y_start + y] = color
            elif pattern_pos < block_width * 2 + gap_width * 2:  # Second gap
                pixels[x, y_start + y] = (0, 0, 0)
            elif pattern_pos < block_width * 3 + gap_width * 2:  # Third block
                pixels[x, y_start + y] = color
            else:  # Third gap
                pixels[x, y_start + y] = (0, 0, 0)

img.save('../src/ncat/rainbow.png')
print("Rainbow created: ../src/ncat/rainbow.png")
