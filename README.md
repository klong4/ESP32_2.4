# Teensy 4.0 Display System with ST7789VI & FT5x26

PlatformIO project ported to Teensy 4.0 (600MHz IMXRT1062) with ST7789VI TFT display (240x320), FT5x26 capacitive touch controller, and SD card support. Features separate SPI buses for display and storage with bit-banged SPI for display compatibility.

## Hardware Configuration

### Display (ST7789VI) - Bit-Banged SPI (SPI0 Pins)
- **DC**: GPIO 9 (Data/Command Select)
- **CS**: GPIO 10 (Chip Select)
- **MOSI**: GPIO 11 (Serial Data In)
- **SCK**: GPIO 13 (SPI Clock)
- **MISO**: GPIO 12 (Serial Data Out, optional)
- **RST**: GPIO 8 (Reset, shared with touch)
- **BL**: GPIO 6 (Backlight PWM control)
- **TE**: GPIO 2 (Tearing Effect / vsync, optional)

**Interface Mode Selection:**
- **IM[3:0] = 1101b** (4-wire SPI with separate DC pin)
  - IM0 (Pin 31): 3.3V (HIGH)
  - IM1 (internal): Pulled LOW
  - IM2 (Pin 32): 3.3V (HIGH)
  - IM3 (internal): Pulled LOW

### SD Card - Hardware SPI1
- **CS**: GPIO 0 (Chip Select)
- **MOSI**: GPIO 26 (SPI1 MOSI)
- **MISO**: GPIO 1 (SPI1 MISO)
- **SCK**: GPIO 27 (SPI1 Clock)
- **Clock**: 25 MHz

### Touch Controller (FT5x26) - I2C
- **SDA**: GPIO 18 (I2C Data)
- **SCL**: GPIO 19 (I2C Clock)
- **INT**: GPIO 7 (Touch Interrupt, optional)
- **RST**: GPIO 8 (Reset, shared with display)
- **Clock**: 400 kHz
- **Address**: 0x38 (7-bit)

> **Key Design**: Display uses bit-banged SPI for ST7789VI compatibility. SD card uses dedicated hardware SPI1 bus for performance. No bus conflicts.

## Features

### Hardware Integration
- ✅ ST7789VI display driver with bit-banged SPI protocol
- ✅ FT5x26 capacitive touch (2-point multi-touch)
- ✅ SD card on dedicated SPI1 bus (SdFat library)
- ✅ 240×320 portrait orientation
- ✅ RGB666 color format (18-bit, 262K colors)
- ✅ CS toggling after every byte (Newhaven protocol)
- ✅ Touch coordinates with event detection
- ✅ File operations on SD card verified

### Display & Performance
- ✅ Bit-banged SPI for ST7789VI compatibility
- ✅ 1μs delays between clock transitions
- ✅ RGB565→RGB666 color conversion
- ✅ Full-screen color tests working
- ✅ Backlight PWM control
- ✅ Hardware reset sequence

### Memory & Storage
- ✅ Separate SPI buses eliminate conflicts
- ✅ SD card file read/write verified
- ✅ Pre-existing image files detected
- ✅ Test file creation successful

## Building and Uploading

### Prerequisites
- PlatformIO IDE (VS Code extension) or PlatformIO Core
- Teensy 4.0 development board
- USB cable for programming and power
- Teensy Loader (auto-installed by PlatformIO)

### Build Commands

```bash
# Build project
pio run

# Upload to Teensy 4.0
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200

# Build and upload in one command
pio run --target upload
```

### VS Code
1. Open project folder in VS Code
2. Click the PlatformIO icon in the sidebar
3. Under "Project Tasks", select:
   - **Build** to compile
   - **Upload** to flash (may need to press button on Teensy)
   - **Monitor** to view serial output

## Project Structure

```
ESP32_2.4_Teensy40_Port/
├── platformio.ini              # PlatformIO configuration (Teensy 4.0)
├── src/
│   ├── main.cpp                # Main application (display, touch, SD tests)
│   ├── boot_splash_data.c      # Embedded boot splash (from ESP32 version)
│   └── CMakeLists.txt          # Build config
├── lib/
│   ├── ST7789/
│   │   ├── st7789.h            # Display driver header
│   │   └── st7789.cpp          # Bit-banged SPI driver
│   ├── FT5x26/
│   │   ├── ft6236.h            # Touch controller header (renamed for FT5x26)
│   │   └── ft6236.cpp          # Teensy Wire library implementation
│   ├── SD/
│   │   ├── sd_spi.h            # SD card driver header (not used)
│   │   └── sd_spi.c            # SD card driver (ESP32, not used)
│   └── LVGL_PORT/
│       ├── lvgl_port.h         # LVGL adapter (not yet integrated)
│       └── lvgl_port.c         # LVGL integration (not yet integrated)
├── data/
│   ├── nyan_0.raw - nyan_11.raw  # Screensaver frames (on SD card)
│   ├── hpt_logo.raw            # HPT logo
│   └── boot_splash.raw         # Boot splash
├── tools/
│   └── convert_images.py       # Image conversion utilities
├── lv_conf.h                   # LVGL configuration
├── TEENSY_3WIRE_SPI_PINOUT.md  # Detailed pinout documentation
└── README.md                   # This file
```

## Hardware Setup

### Wiring Connections

**Display (ST7789VI) - ER-TFTM024-3:**
| Display Pin | Function | Teensy 4.0 | Notes |
|------------|----------|------------|-------|
| 1 | GND | GND | Ground |
| 6 | SDO (MISO) | 12 | Optional read |
| 7 | VDD | 3.3V | Power |
| 8 | VDDI | 3.3V | Logic power |
| 9 | SDA (MOSI) | 11 | Data |
| 10 | CSX | 10 | Chip select |
| 11 | DCX | 9 | Data/Command |
| 12 | WRX (SCK) | 13 | Clock |
| 13 | RDX | 3.3V | Tie HIGH |
| 30 | RESX | 8 | Reset |
| 31 | IM0 | 3.3V | Mode select |
| 32 | IM2 | 3.3V | Mode select |
| 34-37 | LED-K1-4 | GND | Backlight cathode |
| 38 | LED-A | 6 (via resistor) | Backlight anode |
| 40 | TE | 2 | Vsync (optional) |

**Touch Panel (FT5x26):**
| CTP Pin | Function | Teensy 4.0 | Notes |
|---------|----------|------------|-------|
| 1 | VDD | 3.3V | Power |
| 2 | VSS | GND | Ground |
| 3 | SCL | 19 | I2C clock (4.7kΩ pullup) |
| 4 | SDA | 18 | I2C data (4.7kΩ pullup) |
| 5 | /INT | 7 | Interrupt (optional) |
| 6 | /RESET | 8 | Reset (shared) |

**SD Card (SPI1):**
| SD Pin | Function | Teensy 4.0 | Notes |
|--------|----------|------------|-------|
| CS | Chip Select | 0 | Dedicated CS |
| MOSI | Data In | 26 | SPI1 MOSI |
| MISO | Data Out | 1 | SPI1 MISO |
| SCK | Clock | 27 | SPI1 SCK |
| VCC | Power | 3.3V | Power |
| GND | Ground | GND | Ground |

### Important Notes
1. **Backlight Resistor**: Use 100-150Ω resistor in series with LED-A (Pin 38) for ~20mA current
2. **I2C Pull-ups**: 4.7kΩ pull-up resistors recommended on SDA and SCL lines
3. **Power**: All connections use 3.3V (NOT 5V)
4. **SD Card Format**: FAT32 recommended for compatibility

## Current Status

### ✅ Working Components
- **Display**: Backlight control confirmed working
- **Touch**: I2C communication verified, coordinates detected, multi-touch working
- **SD Card**: File operations successful on SPI1, existing files detected

### ⚠️ In Progress
- **Display Image Output**: Backlight on but image output not yet confirmed
  - Initialization sequence matches Newhaven reference
  - CS toggling protocol implemented (toggles after every byte)
  - RGB565→RGB666 conversion in place
  - Display ON command sent after pixel write
  - Need to verify: actual pixel data reaching display

### 🔧 Troubleshooting Steps Completed
1. ✅ Converted ILI9341 driver → ST7789VI
2. ✅ Switched to bit-banged SPI (manual GPIO control)
3. ✅ Implemented CS toggle after each byte (Newhaven protocol)
4. ✅ Applied 18-bit RGB666 color mode
5. ✅ Matched MADCTL settings from SPI sample (0x88)
6. ✅ Moved Display ON command after first pixel write
7. ✅ Verified interface mode: IM[3:0] = 1101b (4-wire SPI)
8. ✅ Separated SD card onto SPI1 to eliminate bus conflicts

### 📋 Next Steps
- Verify display shows color sequences (WHITE→RED→GREEN→BLUE→BLACK→checkerboard)
- Oscilloscope verification of SPI signals if needed
- LVGL integration once display output confirmed
- Implement screensaver with SD card images

## Usage

### Boot Sequence
1. **Initialization**: Display, touch, and SD card initialize
2. **Display Test**: Cycles through colors (2 seconds each)
3. **Touch Test**: 5 seconds to test touch coordinates
4. **SD Card Test**: File operations and directory listing
5. **Idle Loop**: Touch monitoring disabled by default

### Serial Monitor Output
```
========================================
Teensy 4.0 ST7789VI + FT5x26 Touch Demo
========================================

Testing backlight...
Display initialized successfully

*** DISPLAY TEST - Watch the screen for color changes ***
Expected: Full screen color changes every 2 seconds
WHITE -> RED -> GREEN -> BLUE -> BLACK -> Checkerboard

Filling screen with WHITE...
Turning on display (sending command 0x29)...
...

Testing FT5x26 Touch Controller
I2C device found at address 0x38
Touch coordinates: X=166, Y=167

Testing SD Card
SD card initialized successfully!
Root directory contents:
  nyan_0.raw through nyan_11.raw
  boot_splash.raw
  test.txt
```

## API Reference

### Display Functions (lib/ST7789)
```cpp
bool st7789_init(const st7789_config_t *config);
void st7789_fill_screen(uint16_t color);
void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7789_display_on(void);
void st7789_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
```

### Touch Functions (lib/FT5x26)
```cpp
bool ft5x26_init(const ft5x26_config_t *config);
bool ft5x26_read_touch(ft5x26_touch_t *touch_data);
bool ft5x26_is_touched(void);
```

### SD Card (SdFat Library)
```cpp
// Initialize SD card on SPI1
SPI1.setMOSI(SD_MOSI);
SPI1.setMISO(SD_MISO);
SPI1.setSCK(SD_SCK);
SD.begin(SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(25), &SPI1));

// File operations
testFile.open("filename.txt", O_RDONLY);
testFile.read();
```

## Technical Details

### Display Protocol (ST7789VI)
- **SPI Mode**: Bit-banged (manual GPIO toggling)
- **Clock**: Starts LOW, pulses HIGH (rising edge trigger)
- **CS Protocol**: Toggle HIGH after EVERY byte (command or data)
- **DC Pin**: LOW = command, HIGH = data
- **Color Format**: RGB666 (18-bit per pixel, 3 bytes)
- **Conversion**: RGB565 → RGB666 (5→8 bit R, 6→8 bit G, 5→8 bit B)
- **Delays**: 1μs between clock edges

### Touch Protocol (FT5x26)
- **Interface**: I2C at 400kHz
- **Register Read**: 13 bytes from address 0x00
- **Touch Points**: Up to 2 simultaneous touches
- **Coordinates**: 12-bit resolution (0-4095)
- **Event Types**: Press Down (0), Lift Up (1), Contact (2), No Event (3)

### SD Card Protocol
- **Interface**: Hardware SPI1 (SdFat library)
- **Clock**: 25 MHz
- **Format**: FAT32 recommended
- **File Access**: Buffered reads/writes
- **Bus**: Completely separate from display (no conflicts)

## Troubleshooting

### Display shows backlight but no image
**Status**: Current issue being investigated

**Verified:**
- ✅ Backlight working (Pin 6 PWM control)
- ✅ Interface mode correct (IM[3:0] = 1101b)
- ✅ Initialization sequence matches Newhaven SPI sample
- ✅ CS toggling after every byte implemented
- ✅ 18-bit color mode configured

**Try:**
1. Verify all power rails at 3.3V (VDD, VDDI, IM0, IM2)
2. Check SPI connections with oscilloscope
3. Verify IM1 and IM3 are pulled LOW (confirm interface mode)
4. Test with different MADCTL values (0x00, 0x08, 0xA0, 0xC0)

### Touch not responding
**Status**: Working - coordinates detected

If issues occur:
- Verify I2C address: 0x38 (7-bit)
- Check pull-up resistors on SDA/SCL (4.7kΩ)
- Verify 3.3V power to touch controller
- Check shared reset line (Pin 8)

### SD card not detected
**Status**: Working - files detected and created

If issues occur:
- Verify SPI1 connections (CS=0, MOSI=26, MISO=1, SCK=27)
- Format card as FAT32
- Try different SD card
- Check 3.3V power to card

### Serial monitor shows old output
**Status**: Upload may not have completed

**Fix:**
- Close serial monitor before uploading
- Press physical button on Teensy during upload if needed
- Wait for "SUCCESS" message before monitoring

## Known Issues & Limitations

1. **Display Output**: Backlight confirmed but pixel output not yet verified
2. **LVGL Integration**: Not yet ported from ESP32 version
3. **Screensaver**: Waiting for display output verification
4. **Touch Values**: Initial reads show 4095/4095 (no-touch state) - normal behavior

## Future Enhancements

- [ ] Confirm display pixel output and color rendering
- [ ] Port LVGL integration from ESP32 version  
- [ ] Implement screensaver with Nyan Cat animation
- [ ] Add boot splash screen
- [ ] Integrate cable selection UI
- [ ] Optimize SPI timing for better performance
- [ ] Add DMA support if beneficial

## Differences from ESP32 Version

| Feature | ESP32-S3 | Teensy 4.0 |
|---------|----------|------------|
| **MCU** | ESP32-S3 @ 240MHz | IMXRT1062 @ 600MHz |
| **Display** | ILI9341 | ST7789VI |
| **Display SPI** | Hardware SPI2 @ 80MHz | Bit-banged @ ~1MHz |
| **Touch** | FT6236 | FT5x26 |
| **SD Card** | Hardware SPI3 @ 20MHz | Hardware SPI1 @ 25MHz |
| **I2C Library** | ESP-IDF driver | Teensy Wire library |
| **SD Library** | ESP-IDF VFS FAT | SdFat |
| **Bus Sharing** | Display + SD shared | Completely separate |
| **CS Protocol** | Held LOW during transaction | Toggle after each byte |
| **Color Format** | RGB565 with Swap+Invert | RGB666 (18-bit) |

## License

Sample code for display system project. Modify and use as needed.

## Repository

GitHub: [klong4/ESP32_2.4](https://github.com/klong4/ESP32_2.4)  
Branch: `teensy40-port`

## Credits

- **Display Driver**: Based on Newhaven ST7789 SPI sample code
- **Touch Driver**: FT5x26 based on FT6236 implementations
- **Platform**: Teensy 4.0 by PJRC
- **Libraries**: Arduino, Wire, SPI, SdFat
- **Original ESP32 Version**: ILI9341 + FT6236 cable tester UI


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
