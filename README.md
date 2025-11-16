# ESP32-S3 HPTuners Cable Tester UI

PlatformIO project for ESP32-S3 with ILI9341 TFT display (320x240) and FT6236 capacitive touch controller. Features LVGL-based roller UI for cable selection, animated Nyan Cat screensaver, and boot splash screen.

## Hardware Configuration

### Display (ILI9341) - SPI2 Interface
- **MOSI**: GPIO 11
- **MISO**: GPIO 13
- **SCLK**: GPIO 12
- **CS**: GPIO 10
- **DC**: GPIO 9
- **RST**: GPIO 14
- **BL** (Backlight): GPIO 15
- **Clock**: 80 MHz (optimized for maximum throughput)

### SD Card - SPI3 Interface
- **MOSI**: GPIO 35
- **MISO**: GPIO 37
- **SCLK**: GPIO 36
- **CS**: GPIO 34
- **Clock**: 20 MHz (stable maximum speed)

### Touch Controller (FT6236) - I2C Interface
- **SDA**: GPIO 4
- **SCL**: GPIO 5
- **INT**: GPIO 8 (not currently used - polling mode)
- **Clock**: 400 kHz

> **Note**: Adjust pin assignments in `src/main.c` to match your wiring.

## Features

### User Interface
- ✅ LVGL 9.4-based touch UI
- ✅ Roller widget for cable selection (8 cable types)
- ✅ Top status bar showing currently selected cable
- ✅ Color profile cycling (5 profiles, changes every 5 seconds)
- ✅ Touch-responsive navigation
- ✅ Boot splash with embedded HPTuners logo (no SD card required)

### Screensaver
- ✅ Nyan Cat animated screensaver (12 frames)
- ✅ Activates after 10 seconds of inactivity
- ✅ Touch to exit and return to UI
- ✅ Optimized animation with pre-transformed images
- ✅ Double-buffered rendering for smooth playback
- ✅ SPI optimizations: 16-deep queue, 32KB chunks, queued DMA transfers

### Display & Performance
- ✅ ILI9341 driver with SPI optimizations
- ✅ 320×240 landscape orientation
- ✅ RGB565 color format with Swap+Invert transformation
- ✅ Hardware DMA support
- ✅ Optimized SPI transfers (80MHz display, 20MHz SD card)
- ✅ Persistent file handles for screensaver frames
- ✅ Pre-converted images eliminate runtime color transformation

### Memory Management
- ✅ RAM usage: 48.1% (157,484 / 327,680 bytes)
- ✅ Flash usage: 81.7% (857,173 / 1,048,576 bytes)
- ✅ Double buffering for screensaver (2 × 25,600 bytes)
- ✅ Regular malloc (not DMA) to preserve USB functionality

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
├── platformio.ini              # PlatformIO configuration
├── src/
│   ├── main.c                  # Main application (UI, screensaver, cable detection)
│   ├── boot_splash_data.c      # Embedded boot splash image (auto-generated)
│   └── CMakeLists.txt          # ESP-IDF build config
├── lib/
│   ├── ILI9341/
│   │   ├── ili9341.h           # Display driver header
│   │   └── ili9341.c           # Display driver with SPI optimizations
│   ├── FT6236/
│   │   ├── ft6236.h            # Touch controller header
│   │   └── ft6236.c            # Touch controller implementation
│   ├── SD/
│   │   ├── sd_spi.h            # SD card driver header
│   │   └── sd_spi.c            # SD card driver (SPI3, 20MHz)
│   └── LVGL_PORT/
│       ├── lvgl_port.h         # LVGL display/input adapter
│       └── lvgl_port.c         # LVGL integration layer
├── data/
│   ├── nyan_0.raw - nyan_11.raw  # Pre-transformed screensaver frames
│   └── boot_splash.raw         # Boot splash (also embedded in firmware)
├── tools/
│   ├── convert_nyan.py         # Convert Nyan Cat frames (Swap+Invert)
│   ├── convert_boot_logo.py    # Convert boot splash (Swap+Invert)
│   └── embed_boot_splash.py    # Embed boot splash into firmware
├── lv_conf.h                   # LVGL configuration
└── README.md                   # This file
```

## SD Card Setup

Copy the following files to your SD card root:
- `data/nyan_0.raw` through `data/nyan_11.raw` (screensaver animation)

The boot splash is embedded in firmware and doesn't require SD card files.

## Image Pre-Processing

All images use **Swap+Invert** transformation for the ILI9341 display:

```python
# RGB888 → RGB565
rgb565 = (r5 << 11) | (g6 << 5) | b5

# Byte swap (endianness)
swapped = (rgb565 >> 8) | ((rgb565 & 0xFF) << 8)

# Invert (display-specific quirk)
final = ~swapped & 0xFFFF
```

### Regenerate Images
```bash
# Screensaver frames (requires src/ncat/full frame/*.png)
python convert_nyan.py

# Boot splash (requires src/ncat/HPT.png)
python convert_boot_logo.py

# Embed boot splash into firmware
python embed_boot_splash.py
```

## Usage

### Boot Sequence
1. **Boot Splash**: HPTuners logo displays (embedded in firmware)
2. **Touch to Start**: Touch the screen to proceed to main UI
3. **Main UI**: Roller interface with cable selection

### Main UI Operation
- **Scroll**: Swipe up/down to navigate cable options
- **Select**: Tap a cable name to select it
- **Top Bar**: Shows currently selected cable
- **Color Profiles**: UI colors cycle automatically every 5 seconds

### Screensaver
- **Activation**: After 10 seconds of inactivity
- **Animation**: Nyan Cat loops continuously
- **Exit**: Touch anywhere on screen to return to UI

### Cable Options
1. HPT Standard
2. HPT Pro
3. HPT Pro+
4. MPVI2
5. MPVI2+
6. MPVI3
7. VCM Suite
8. Custom Cable

## Performance Optimizations

### Display SPI (80 MHz)
- Queue size: 16 transactions
- Chunk size: 32 KB per transfer
- Transfer mode: Queued DMA
- Flags: `SPI_DEVICE_NO_DUMMY`

### SD Card SPI (20 MHz)
- Transfer buffer: 65 KB
- Chunk size: 40 lines (25,600 bytes)
- Note: 40 MHz causes read failures

### Screensaver Rendering
- **Pre-transformation**: Color conversion done during image creation
- **Persistent handles**: File stays open during frame display
- **Double buffering**: Concurrent SD read + display write
- **Zero frame delay**: Maximum animation speed
- **File size**: 153,600 bytes per frame (320×240 RGB565)

## Customization

### Screensaver Timeout
Edit `src/main.c`:
```c
#define SCREENSAVER_TIMEOUT_MS 10000  // Change to desired milliseconds
```

### Color Profile Cycle Time
Edit `src/main.c`:
```c
if (current_time - last_profile_change > 5000) {  // Change 5000 to desired ms
```

### Add Cable Types
Edit the roller options in `src/main.c`:
```c
lv_roller_set_options(roller,
    "HPT Standard\n"
    "HPT Pro\n"
    // ... add more cables here
```

### Animation Frame Rate
The screensaver runs at maximum speed (no delay). To add delay:
```c
#define FRAME_DELAY_MS 0  // Change to desired milliseconds per frame
```

## Troubleshooting

### Boot splash shows wrong colors
- Regenerate with: `python convert_boot_logo.py && python embed_boot_splash.py`
- Ensure Swap+Invert transformation is applied
- Re-upload firmware

### Screensaver not activating
- Check SD card is inserted and files are present
- Verify files: `nyan_0.raw` through `nyan_11.raw`
- Monitor serial output for SD card errors
- Default timeout: 10 seconds of no touch

### Screensaver choppy/slow
- Current optimizations should provide smooth playback
- Check SD card speed (Class 10 recommended)
- Verify SPI clock is 20 MHz (higher speeds may fail)

### Touch not responding
- Check I2C connections (SDA=GPIO4, SCL=GPIO5)
- Verify 3.3V power to FT6236
- Monitor serial for I2C errors
- Try adding 4.7kΩ pull-ups to SDA/SCL

### Display issues
- Verify SPI2 connections (MOSI=11, SCLK=12, CS=10)
- Check backlight (GPIO 15 should be HIGH)
- Confirm 3.3V power to display
- Try reducing SPI clock if artifacts appear

### SD card not detected
- Verify SPI3 connections (MOSI=35, MISO=37, SCLK=36, CS=34)
- Check card format (FAT32 recommended)
- Ensure card is properly inserted
- Try different SD card

### USB stops working after upload
- Current code uses regular malloc (not DMA) to avoid this
- If issue persists, check for memory corruption
- Verify RAM usage is under 50%

## API Reference

### Display Functions (lib/ILI9341)
```c
bool ili9341_init(const ili9341_config_t *config);
void ili9341_fill_screen(uint16_t color);
void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ili9341_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ili9341_write_pixels(const uint16_t* pixels, uint32_t length);
void ili9341_set_backlight(uint8_t brightness);
```

### Touch Functions (lib/FT6236)
```c
bool ft6236_init(const ft6236_config_t *config);
bool ft6236_read_touch(ft6236_touch_t *touch_data);
bool ft6236_is_touched(void);
```

### SD Card Functions (lib/SD)
```c
bool sd_init(int cs_pin, int mosi_pin, int miso_pin, int clk_pin);
bool sd_mount(void);
void sd_unmount(void);
bool sd_read_chunk(const char* filename, uint32_t offset, uint8_t* buffer, uint32_t size);
```

### LVGL Port (lib/LVGL_PORT)
```c
bool lvgl_port_init(void);
void lvgl_port_task(void);
```

## Technical Details

### Color Format
- **Format**: RGB565 (16-bit per pixel)
- **Transformation**: Swap + Invert
- **Byte order**: Little-endian after swap
- **Total colors**: 65,536 (5-bit R, 6-bit G, 5-bit B)

### Memory Layout
- **Screensaver buffers**: 2 × 25,600 bytes (40 lines × 320 pixels × 2 bytes)
- **LVGL buffers**: Configured in `lv_conf.h`
- **Stack allocation**: Regular malloc (not DMA)
- **File handles**: Persistent during frame display

### Timing
- **Screensaver activation**: 10 seconds idle
- **Color profile change**: 5 seconds
- **Touch polling**: 50 ms intervals
- **LVGL task period**: 10 ms
- **Animation frame delay**: 0 ms (maximum speed)

## Known Issues & Limitations

1. **SD Card Speed**: 40 MHz SPI causes read failures; limited to 20 MHz
2. **DMA Memory**: Using DMA-capable memory breaks USB-Serial/JTAG
3. **Flash Size Warning**: PlatformIO reports 2MB but board has 8MB (cosmetic issue)
4. **Cable ID Detection**: Currently placeholder implementation

## Future Enhancements

- [ ] Implement actual cable ID detection via GPIO
- [ ] Add USB data passthrough functionality
- [ ] Store last selected cable in NVS
- [ ] Add settings menu for timeout/colors
- [ ] Implement firmware update via USB
- [ ] Add cable diagnostics display

## License

Sample code for HPTuners cable tester project. Modify and use as needed.

## Credits

- **Display Driver**: Based on ILI9341 reference implementations
- **Touch Driver**: FT6236 from ER-TFTM024-3 examples
- **LVGL**: Open-source embedded GUI library (v9.4)
- **Nyan Cat**: Classic meme animation
- **Optimization**: Extensive performance tuning for smooth animation

## Repository

GitHub: [klong4/ESP32_2.4](https://github.com/klong4/ESP32_2.4)
