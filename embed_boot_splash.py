#!/usr/bin/env python3
"""Embed boot_splash.raw into firmware as C array"""

import os

def embed_file_as_array(input_path, output_path, array_name):
    """Convert binary file to C array"""
    with open(input_path, 'rb') as f:
        data = f.read()
    
    with open(output_path, 'w') as f:
        f.write(f'// Auto-generated from {input_path}\n')
        f.write(f'// File size: {len(data)} bytes\n\n')
        f.write(f'#include <stdint.h>\n\n')
        f.write(f'const uint32_t {array_name}_size = {len(data)};\n')
        f.write(f'const uint8_t {array_name}[] = {{\n')
        
        for i in range(0, len(data), 16):
            chunk = data[i:i+16]
            hex_values = ', '.join(f'0x{b:02x}' for b in chunk)
            f.write(f'    {hex_values},\n')
        
        f.write('};\n')
    
    print(f"Embedded {len(data)} bytes into {output_path}")

embed_file_as_array('data/boot_splash.raw', 'src/boot_splash_data.c', 'boot_splash_data')
