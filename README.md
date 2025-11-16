# ESP32-S3 ILI9341 + FT6236 Touch Display

PlatformIO project for ESP32-S3 with ILI9341 TFT display (240x320) and FT6236 capacitive touch controller.

## Hardware Configuration

### Display (ILI9341) - SPI Interface
- **MOSI**: GPIO 11
- **MISO**: GPIO 13 (optional, not used for write-only)
- **SCLK**: GPIO 12
- **CS**: GPIO 10
- **DC**: GPIO 9
- **RST**: GPIO 14
- **BL** (Backlight): GPIO 15

### Touch Controller (FT6236) - I2C Interface
- **SDA**: GPIO 4
- **SCL**: GPIO 5
- **INT**: Not connected (polling mode)

> **Note**: Adjust pin assignments in `src/main.c` to match your wiring.

## Features

- ✅ ILI9341 display driver with full initialization sequence
- ✅ FT6236 capacitive touch controller support
- ✅ Multi-touch support (up to 2 simultaneous touch points)
- ✅ Real-time touch visualization
- ✅ Gradient background demo
- ✅ RGB565 color support
- ✅ Hardware SPI with DMA support
- ✅ Modular driver architecture

## Building and Uploading

### Prerequisites
- PlatformIO IDE (VS Code extension) or PlatformIO Core
- ESP32-S3 development board
- USB cable for programming and power

### Build Commands

```bash
# Build project
pio run

# Upload to ESP32-S3
pio run --target upload

# Monitor serial output
pio device monitor

# Build, upload, and monitor in one command
pio run --target upload && pio device monitor
```

### VS Code
1. Open project folder in VS Code
2. Click the PlatformIO icon in the sidebar
3. Under "Project Tasks", select:
   - **Build** to compile
   - **Upload** to flash
   - **Monitor** to view serial output

## Project Structure

```
ESP32-S3_ILI9341_FT6236/
├── platformio.ini          # PlatformIO configuration
├── src/
│   └── main.c              # Main application code
├── lib/
│   ├── ILI9341/
│   │   ├── ili9341.h       # Display driver header
│   │   └── ili9341.c       # Display driver implementation
│   └── FT6236/
│       ├── ft6236.h        # Touch controller header
│       └── ft6236.c        # Touch controller implementation
└── README.md               # This file
```

## Usage

After uploading:
1. The display shows a colorful gradient background
2. Touch the screen to draw circles:
   - First touch point → **RED** circle
   - Second touch point (multi-touch) → **GREEN** circle
3. Touch coordinates are logged to serial monitor
4. After 5 seconds of no touch, screen auto-clears

## Customization

### Pin Configuration
Edit `src/main.c` to change pin assignments:
```c
#define TFT_MOSI    11
#define TFT_SCLK    12
// ... etc
```

### Display Orientation
Modify `MADCTL` register in `ili9341.c` initialization:
```c
ili9341_send_cmd(ILI9341_MADCTL);
ili9341_send_u8(0x08);  // Change value for different orientations
```

Common values:
- `0x08` - Portrait
- `0x68` - Landscape
- `0xC8` - Portrait rotated 180°
- `0xA8` - Landscape rotated 180°

### Touch Coordinate Mapping
If touch coordinates don't match screen position, adjust in `src/main.c`:
```c
// Example: swap X/Y or invert axes
uint16_t corrected_x = ILI9341_WIDTH - touch_data.points[0].x;
```

## Troubleshooting

### Display shows nothing
- Check power supply (3.3V or 5V depending on module)
- Verify SPI pin connections
- Check backlight pin (GPIO 15) is HIGH
- Confirm CS and RST pins

### Touch not working
- Verify I2C address (0x38 for FT6236)
- Check SDA/SCL connections
- Ensure 3.3V power to touch controller
- Add external 4.7kΩ pull-up resistors to SDA/SCL if needed
- Monitor I2C communication errors in serial output

### Wrong touch coordinates
- Adjust coordinate mapping in main.c
- Check display orientation matches touch orientation
- Swap X/Y axes if needed

### SPI Errors
- Reduce SPI clock speed in `src/main.c`:
  ```c
  .spi_clock_mhz = 20  // Try lower speed
  ```

## API Reference

### Display Functions
```c
bool ili9341_init(const ili9341_config_t *config);
void ili9341_fill_screen(uint16_t color);
void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
```

### Touch Functions
```c
bool ft6236_init(const ft6236_config_t *config);
bool ft6236_read_touch(ft6236_touch_t *touch_data);
bool ft6236_is_touched(void);
```

## License

This is sample code for educational purposes. Modify and use as needed for your projects.

## Credits

Based on ILI9341 and FT6236 reference implementations from ER-TFTM024-3 example code.
