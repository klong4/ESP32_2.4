# Teensy 4.0 Port - Work in Progress

This is a fork of the ESP32-S3 HPTuners Cable Tester project, being ported to Teensy 4.0.

## Port Status: 🚧 In Progress

### Target Hardware
- **MCU**: Teensy 4.0 (600 MHz ARM Cortex-M7, NXP i.MX RT1062)
- **Display**: [Specify your display model here]
- **Touch**: [Specify your touch controller here]
- **Storage**: [SD card / SPI Flash / etc.]

## Key Differences from ESP32-S3

### Hardware Capabilities
| Feature | ESP32-S3 | Teensy 4.0 |
|---------|----------|------------|
| CPU | 240 MHz Xtensa | 600 MHz ARM Cortex-M7 |
| RAM | 320 KB SRAM + 2MB PSRAM | 1024 KB RAM |
| Flash | 8 MB | 2 MB onboard |
| SPI | 2x SPI (80 MHz display, 20 MHz SD) | 3x SPI (up to 50 MHz) |
| I2C | 2x I2C | 3x I2C |
| GPIO | 3.3V | 3.3V |
| Framework | ESP-IDF | Arduino/Teensyduino |

### Required Changes

#### 1. Build System
- [ ] Replace `platformio.ini` ESP-IDF configuration with Teensy/Arduino setup
- [ ] Update build flags and dependencies
- [ ] Configure Teensyduino board settings

#### 2. Display Driver (lib/ILI9341 or new driver)
- [ ] Port SPI initialization to Teensy SPI library
- [ ] Adjust pin mappings
- [ ] Update DMA/transfer methods
- [ ] Test color transformation (Swap+Invert may differ)

#### 3. Touch Controller (lib/FT6236 or new driver)
- [ ] Port I2C initialization to Teensy Wire library
- [ ] Update pin assignments
- [ ] Test touch coordinate mapping

#### 4. LVGL Integration
- [ ] Update LVGL port for Teensy
- [ ] Configure display buffer allocation
- [ ] Adjust timing for 600 MHz CPU
- [ ] Update `lv_conf.h` for Teensy memory layout

#### 5. Storage (SD Card)
- [ ] Port SD library to Teensy SD library
- [ ] Update SPI bus configuration
- [ ] Test file read performance
- [ ] May need different speed settings

#### 6. Boot Splash
- [ ] Embedded image may need different linker sections
- [ ] Update embedding script for Teensy toolchain
- [ ] Test flash storage availability

#### 7. Application Code
- [ ] Replace ESP-IDF specific functions (esp_timer, vTaskDelay, etc.)
- [ ] Port to Arduino millis()/delay()
- [ ] Update logging (ESP_LOGI → Serial.print)
- [ ] Adjust memory allocation (malloc should work, but test)

## Suggested Pin Mapping (Example - Adjust for Your Hardware)

### Display SPI
- **MOSI**: Pin 11 (MOSI0)
- **MISO**: Pin 12 (MISO0) 
- **SCLK**: Pin 13 (SCK0)
- **CS**: Pin 10
- **DC**: Pin 9
- **RST**: Pin 8
- **BL**: Pin 7 (PWM capable for brightness control)

### Touch I2C
- **SDA**: Pin 18 (SDA0/Wire)
- **SCL**: Pin 19 (SCL0/Wire)
- **INT**: Pin 2 (interrupt capable)

### SD Card SPI
- **MOSI**: Pin 26 (MOSI1)
- **MISO**: Pin 39 (MISO1)
- **SCLK**: Pin 27 (SCK1)
- **CS**: Pin 38

## Development Steps

### Phase 1: Basic Display
1. Port ILI9341 driver or implement new display driver
2. Get basic shapes/fills working
3. Verify color output

### Phase 2: Touch Input
1. Port or implement touch controller driver
2. Test touch detection
3. Calibrate coordinate mapping

### Phase 3: LVGL UI
1. Integrate LVGL with Teensy
2. Port roller UI and color profiles
3. Test touch responsiveness

### Phase 4: Storage & Assets
1. Implement SD card access
2. Port screensaver frame loading
3. Test animation performance

### Phase 5: Complete Features
1. Port boot splash
2. Implement screensaver timeout
3. Add cable detection logic

## Performance Expectations

### Teensy 4.0 Advantages
- **3x faster CPU** (600 MHz vs 240 MHz) - smoother LVGL rendering
- **3x more RAM** (1024 KB vs 320 KB) - larger display buffers possible
- **Faster SPI** (up to 50 MHz) - potentially faster screen updates

### Potential Challenges
- **Less Flash** (2 MB vs 8 MB) - may need external storage for assets
- **No PSRAM** - but base RAM is sufficient
- **Different DMA** - SPI DMA configuration differs from ESP32
- **Arduino Framework** - less control than ESP-IDF but easier to use

## Resources

### Teensy 4.0 Documentation
- [Teensy 4.0 Pinout Card](https://www.pjrc.com/teensy/card11a_rev2.pdf)
- [Teensy 4.0 Forum](https://forum.pjrc.com/)
- [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)

### Libraries
- [LVGL for Arduino](https://github.com/lvgl/lvgl)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) - May be useful for display
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)
- [XPT2046_Touchscreen](https://github.com/PaulStoffregen/XPT2046_Touchscreen) - If using resistive touch

### Display Options for Teensy
- ILI9341 (same as ESP32 version) - 320x240
- ST7789 - 240x320
- ILI9488 - 320x480 (higher res)
- SSD1963 - Large displays up to 800x480

## Next Steps

1. **Choose your display hardware**
2. **Update this file with specific model numbers**
3. **Start with Phase 1: Basic Display**
4. **Test incrementally** - don't try to port everything at once

## Original Project

This is forked from: [ESP32_2.4](https://github.com/klong4/ESP32_2.4)

Original features:
- LVGL roller UI for cable selection
- Nyan Cat screensaver with double buffering
- Boot splash screen
- Color profile cycling
- Optimized SPI transfers

Target: Port all features to Teensy 4.0 platform.

---

**Note**: Remember to update `platformio.ini` or create `platformio.ini` configured for Teensy 4.0, or use Arduino IDE with Teensyduino.

For PlatformIO with Teensy:
```ini
[env:teensy40]
platform = teensy
board = teensy40
framework = arduino
lib_deps = 
    lvgl/lvgl@^9.2.2
```
