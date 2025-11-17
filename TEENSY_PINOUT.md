# Teensy 4.0 Display Pinout Configurations

This document contains pinout configurations for multiple display options supported by this project.

**To select a display configuration:** Edit `platformio.ini` and set the build flag:
- For ST7789 SPI: `-DDISPLAY_ST7789`
- For ST7789 Parallel: `-DDISPLAY_ST7789_PARALLEL`
- For ST7262 Parallel: `-DDISPLAY_ST7262`

## Teensy 4.0 Pin Layout

**Top Side (Easy Access):** Pins 0-23 have standard 0.1" header spacing for easy breadboard prototyping.

**Bottom Side (Solder Pads):** Pins 24-33 are small solder pads on the bottom requiring fine-pitch soldering skills.

**Important:** All display control pins use only top-side pins for easy wiring. Bottom pads are used only for high-speed data lines and I2C when needed to avoid conflicts.

---

# Configuration 1: ST7789VI 240x320 2.4" (SPI Interface)

**Build Flag:** `-DDISPLAY_ST7789`

## Hardware
- **Display**: ST7789VI 240x320 (ER-TFTM024-3)
- **Touch**: FT5x26 Capacitive Touch (I2C0)
- **SD Card**: MicroSD (Hardware SPI1)
- **Interface**: 4-wire SPI with DC pin

## Interface Mode Selection
- **IM0** (Pin 31) = **GPIO controlled (Pin 4)** - Set to HIGH for SPI
- **IM1** (internal) = **Pulled LOW** - Internal pull-down
- **IM2** (Pin 32) = **GPIO controlled (Pin 3)** - Set to HIGH for 4-wire SPI
- **IM3** (internal) = **Pulled LOW** - Internal pull-down

**Resulting mode: IM[3:0] = 1101b = 4-wire SPI with separate DC pin**

Driver automatically sets IM0=HIGH and IM2=HIGH via GPIO pins 4 and 3.

## ST7789 Pin Connections

### TFT Display (ST7789VI) - SPI

| Display Pin | Pin Name | Function | Teensy 4.0 Pin | Notes |
|------------|----------|----------|----------------|-------|
| 1 | GND | Ground | GND | Power ground |
| 6 | SDO | MISO | 12 | Serial Data Out (optional for read) |
| 7 | VDD | Power | 3.3V | Display power supply (3.3V) |
| 8 | VDDI | Logic | 3.3V | Logic level supply (3.3V) |
| 9 | SDA | MOSI | 11 | Serial Data In (SPI MOSI) |
| 10 | CSX | CS | 10 | Chip Select (Active Low) |
| 11 | DCX | DC | 9 | Data/Command Select |
| 12 | WRX | SCK | 13 | SPI Clock (Active Low Write) |
| 13 | RDX | - | 3.3V | Read signal (tie HIGH) |
| 14-29 | DB0-DB15 | - | Not connected | Parallel mode only |
| 30 | RESX | Reset | 8 | Hardware Reset (Active Low) |
| 31 | IM0 | Mode | 4 (GPIO) | Interface mode bit 0 (controlled by Teensy) |
| 32 | IM2 | Mode | 3 (GPIO) | Interface mode bit 2 (controlled by Teensy) |
| 33 | GND | Ground | GND | Ground |
| 34-37 | LED-K1-4 | Backlight | GND | Backlight cathodes |
| 38 | LED-A | Backlight | 6 (PWM) | Backlight anode (via resistor) |
| 39 | GND | Ground | GND | Ground |
| 40 | TE | Sync | 2 (optional) | Tearing Effect (vsync) |

### FT5x26 Capacitive Touch (I2C0)

| Touch Pin | Signal | Teensy 4.0 Pin | Notes |
|-----------|--------|----------------|-------|
| VDD | Power | 3.3V | Touch IC Power |
| GND | Ground | GND | Ground |
| SCL | I2C Clock | 19 (SCL0) | I2C Clock (400kHz) |
| SDA | I2C Data | 18 (SDA0) | I2C Data |
| INT | Interrupt | 17 | Touch Interrupt (optional) |
| RST | Reset | 8 | Touch Reset (shared with display) |

### SD Card (Hardware SPI1)

| SD Pin | Signal | Teensy 4.0 Pin | Notes |
|--------|--------|----------------|-------|
| CS | Chip Select | 0 | SPI1 CS |
| MOSI | Data Out | 26 | SPI1 MOSI (bottom solder pad) |
| MISO | Data In | 1 | SPI1 MISO |
| SCK | Clock | 27 | SPI1 SCK (bottom solder pad, 25MHz) |
| VCC | Power | 3.3V | Power |
| GND | Ground | GND | Ground |

### ST7789 Pin Summary (Teensy 4.0)

**All pins clearly organized by function:**

```
Display Control (Top side):
  CS=10, DC=9, MOSI=11, MISO=12, SCK=13, RST=8, BL=6, TE=2
  
Interface Mode (Top side, GPIO-controlled):
  IM0=4, IM2=3 (automatically set to HIGH for SPI mode)

Touch Controller (Top side):
  I2C0: SDA=18, SCL=19, INT=17, RST=8 (shared with display)

SD Card (Bottom pads required):
  SPI1: CS=0, MOSI=26, MISO=1, SCK=27
```

**Pin Usage:** Display uses only top pins. SD card requires soldering 2 bottom pads (26, 27) for hardware SPI1, or use software SPI with top pins only.

---

# Configuration 2: ST7789VI 240x320 2.4" (8/16-bit Parallel)

**Build Flag:** `-DDISPLAY_ST7789_PARALLEL`

## Hardware
- **Display**: ST7789VI 240x320 (ER-TFTM024-3)
- **Touch**: FT5x26 Capacitive Touch (I2C0)
- **SD Card**: MicroSD (Hardware SPI1)
- **Interface**: 8-bit or 16-bit Parallel

## Interface Mode Selection
- **IM0** (Pin 31) = **GPIO controlled (Pin 4)** - Set to HIGH for 8-bit (DB8-DB15), LOW for 16-bit (DB0-DB15)
- **IM1** (internal) = **Pulled LOW** - Internal pull-down
- **IM2** (Pin 32) = **GPIO controlled (Pin 3)** - Set to LOW for parallel mode
- **IM3** (internal) = **Pulled LOW** - Internal pull-down

**Resulting mode:**
- **8-bit:** IM[3:0] = 0100b (IM0=HIGH, IM2=LOW) - uses DB8-DB15
- **16-bit:** IM[3:0] = 0000b (IM0=LOW, IM2=LOW) - uses DB0-DB15

Driver automatically configures IM0 and IM2 based on selected bit width.

## ST7789 Parallel Pin Connections

### TFT Display (ST7789VI) - 8-bit Parallel

| Display Pin | Pin Name | Function | Teensy 4.0 Pin | Notes |
|------------|----------|----------|----------------|-------|
| 1 | GND | Ground | GND | Power ground |
| 6 | SDO | - | Not connected | Not used in parallel mode |
| 7 | VDD | Power | 3.3V | Display power supply (3.3V) |
| 8 | VDDI | Logic | 3.3V | Logic level supply (3.3V) |
| 9 | SDA | - | Not connected | Not used in parallel mode |
| 10 | CSX | CS | 10 | Chip Select (Active Low) |
| 11 | DCX | DC | 9 | Data/Command Select |
| 12 | WRX | WR | 13 | Write strobe (Active Low) |
| 13 | RDX | RD | 5 | Read strobe (tie HIGH if not reading) |
| 14 | DB0 | Data | 19 | Data Bit 0 (LSB) |
| 15 | DB1 | Data | 18 | Data Bit 1 |
| 16 | DB2 | Data | 14 | Data Bit 2 |
| 17 | DB3 | Data | 15 | Data Bit 3 |
| 18 | DB4 | Data | 28 | Data Bit 4 (bottom solder pad) |
| 19 | DB5 | Data | 29 | Data Bit 5 (bottom solder pad) |
| 20 | DB6 | Data | 30 | Data Bit 6 (bottom solder pad) |
| 21 | DB7 | Data | 31 | Data Bit 7 (MSB for 8-bit, bottom solder pad) |
| 22 | DB8 | Data | 22 | Data Bit 8 (16-bit mode only) |
| 23 | DB9 | Data | 23 | Data Bit 9 (16-bit mode only) |
| 24 | DB10 | Data | 20 | Data Bit 10 (16-bit mode only) |
| 25 | DB11 | Data | 21 | Data Bit 11 (16-bit mode only) |
| 26 | DB12 | Data | 24 | Data Bit 12 (16-bit mode only, bottom solder pad) |
| 27 | DB13 | Data | 25 | Data Bit 13 (16-bit mode only, bottom solder pad) |
| 28 | DB14 | Data | 26 | Data Bit 14 (16-bit mode only, bottom solder pad) |
| 29 | DB15 | Data | 27 | Data Bit 15 (MSB for 16-bit, bottom solder pad) |
| 30 | RESX | Reset | 8 | Hardware Reset (Active Low) |
| 31 | IM0 | Mode | 4 (GPIO) | Interface mode bit 0 (controlled by Teensy) |
| 32 | IM2 | Mode | 3 (GPIO) | Interface mode bit 2 (controlled by Teensy) |
| 33 | GND | Ground | GND | Ground |
| 34-37 | LED-K1-4 | Backlight | GND | Backlight cathodes |
| 38 | LED-A | Backlight | 6 (PWM) | Backlight anode (3.3V compatible) |
| 39 | GND | Ground | GND | Ground |
| 40 | TE | Sync | 2 (optional) | Tearing Effect (vsync) |

### FT5x26 Capacitive Touch (Software I2C)

| Touch Pin | Signal | Teensy 4.0 Pin | Notes |
|-----------|--------|----------------|-------|
| VDD | Power | 3.3V | Touch IC Power |
| GND | Ground | GND | Ground |
| SCL | I2C Clock | 16 | Software I2C Clock (400kHz, top pin) |
| SDA | I2C Data | 17 | Software I2C Data (top pin) |
| INT | Interrupt | 32 | Touch Interrupt (bottom pad) |
| RST | Reset | 8 | Touch Reset (shared with display) |

**NOTE:** Touch uses software I2C on top pins (16, 17) to avoid conflicts with ALL display data pins including 16-bit mode. INT uses bottom pad pin 32.

### SD Card (Software SPI)

| SD Pin | Signal | Teensy 4.0 Pin | Notes |
|--------|--------|----------------|-------|
| CS | Chip Select | 0 | Software SPI CS |
| MOSI | Data Out | 1 | Software SPI MOSI |
| MISO | Data In | 2 | Software SPI MISO |
| SCK | Clock | 7 | Software SPI SCK (4MHz) |
| VCC | Power | 3.3V | Power |
| GND | Ground | GND | Ground |

**NOTE:** Software SPI used to avoid conflicts with display data pins in both 8-bit and 16-bit modes. SCK on pin 7 avoids conflict with IM0 (pin 4).

### Additional Peripherals (Software I2C & ADC)

| Peripheral | Type | Teensy 4.0 Pins | Notes |
|------------|------|-----------------|-------|
| INA219 | I2C Power Monitor | SDA=11, SCL=12 | Software I2C (top side, both have ADC capability) |
| ACS712 | Current Sensor | 33 | ADC1_CH7 (bottom pad) |

**NOTE:** INA219 uses software I2C on pins 11 and 12 which are free in parallel mode. ACS712 analog output connects to pin 33 for ADC reading.

### ST7789 Parallel Pin Summary (Teensy 4.0)

**8-bit mode (Recommended for breadboard):**

```
Display Control (Top side):
  CS=10, DC=9, WR=13, RD=5, RST=8, BL=6

Interface Mode (Top side, GPIO-controlled):
  IM0=4, IM2=3 (automatically set to HIGH/LOW for 8-bit mode using DB8-DB15)

Data Bus:
  Top: DB0=19, DB1=18, DB2=14, DB3=15
  Bottom pads: DB4=28, DB5=29, DB6=30, DB7=31

Touch Controller (Software I2C on top pins):
  Soft I2C: SDA=17, SCL=16, INT=32 (bottom pad), RST=8

SD Card (Software SPI on top pins):
  Soft SPI: CS=0, MOSI=1, MISO=2, SCK=7

Additional Peripherals:
  INA219 (I2C): SDA=11, SCL=12 (software I2C, top side)
  ACS712 (ADC): Pin 33 (ADC1_CH7, bottom pad)
```

**16-bit mode (Requires extensive bottom pad soldering):**

```
Display Control: Same as 8-bit
Interface Mode: IM0=4 (LOW), IM2=3 (LOW) for 16-bit mode

Data Bus (16 pins total):
  Top: DB[19,18,14,15,22,23,20,21]
  Bottom pads: DB[28,29,30,31,24,25,26,27]

Touch: Software I2C (SDA=17, SCL=16, INT=32) - no conflicts
SD Card: Software SPI (CS=0, MOSI=1, MISO=2, SCK=7) - no conflicts

Additional Peripherals:
  INA219 (I2C): SDA=11, SCL=12 (software I2C)
  ACS712 (ADC): Pin 33 (ADC1_CH7, bottom pad)
```

**Recommendation:** Use 8-bit mode to minimize bottom pad soldering and avoid conflicts.

---

# Configuration 3: ST7262 800x480 5" (24-bit Parallel RGB)

**Build Flag:** `-DDISPLAY_ST7262`

## Hardware
- **Display**: ST7262 800x480 5-inch TFT
- **Touch**: FT5x26 Capacitive Touch (I2C1)
- **SD Card**: MicroSD (Software SPI)
- **Interface**: 24-bit Parallel RGB

## ST7262 Pin Connections

### Display (24-bit Parallel RGB)

| Display Pin | Signal | Direction | Teensy 4.0 Pin | Notes |
|-------------|--------|-----------|----------------|-------|
| 1 | LED- | Power | GND | Backlight Ground |
| 2 | LED+ | Power | 23 (PWM) | Backlight (requires 21.7V boost) |
| 3 | GND | Power | GND | Ground |
| 4 | VDD | Power | 3.3V | 3.3V Logic Power |
| 5 | R0 | Input | 19 | Red Data Bit 0 (LSB) |
| 6 | R1 | Input | 18 | Red Data Bit 1 |
| 7 | R2 | Input | 14 | Red Data Bit 2 |
| 8 | R3 | Input | 15 | Red Data Bit 3 |
| 9 | R4 | Input | 28 | Red Data Bit 4 (bottom solder pad) |
| 10 | R5 | Input | 29 | Red Data Bit 5 (bottom solder pad) |
| 11 | R6 | Input | 30 | Red Data Bit 6 (bottom solder pad) |
| 12 | R7 | Input | 31 | Red Data Bit 7 (MSB, bottom solder pad) |
| 13 | G0 | Input | 22 | Green Data Bit 0 (LSB) |
| 14 | G1 | Input | 23 | Green Data Bit 1 |
| 15 | G2 | Input | 20 | Green Data Bit 2 |
| 16 | G3 | Input | 21 | Green Data Bit 3 |
| 17 | G4 | Input | 24 | Green Data Bit 4 (bottom solder pad) |
| 18 | G5 | Input | 25 | Green Data Bit 5 (bottom solder pad) |
| 19 | G6 | Input | 26 | Green Data Bit 6 (bottom solder pad) |
| 20 | G7 | Input | 27 | Green Data Bit 7 (MSB, bottom solder pad) |
| 21 | B0 | Input | 10 | Blue Data Bit 0 (LSB) |
| 22 | B1 | Input | 12 | Blue Data Bit 1 |
| 23 | B2 | Input | 11 | Blue Data Bit 2 |
| 24 | B3 | Input | 13 | Blue Data Bit 3 |
| 25 | B4 | Input | 16 | Blue Data Bit 4 |
| 26 | B5 | Input | 17 | Blue Data Bit 5 |
| 27 | B6 | Input | 32 | Blue Data Bit 6 (bottom solder pad) |
| 28 | B7 | Input | 33 | Blue Data Bit 7 (MSB, bottom solder pad) |
| 29 | GND | Power | GND | Ground |
| 30 | CLKIN | Input | 32 | Pixel Clock (30MHz, bottom solder pad) |
| 31 | STBYB | Input | 33 | Standby (HIGH=Normal, bottom solder pad) |
| 32 | HSD | Input | 4 | Horizontal Sync |
| 33 | VSD | Input | 5 | Vertical Sync |
| 34 | DEN | Input | 6 | Data Enable |
| 35 | NC | - | - | No Connect |
| 36 | GND | Power | GND | Ground |
| 37-40 | XR/YD/XL/YU | - | - | Resistive touch (not used) |

### FT5x26 Capacitive Touch (I2C1)

| Touch Pin | Signal | Teensy 4.0 Pin | Notes |
|-----------|--------|----------------|-------|
| VDD | Power | 3.3V | Touch IC Power |
| GND | Ground | GND | Ground |
| SCL | I2C Clock | 24 (SCL1) | I2C Clock (400kHz, bottom solder pad) |
| SDA | I2C Data | 25 (SDA1) | I2C Data (bottom solder pad) |
| INT | Interrupt | - | Not connected (would conflict with SD SCK) |
| RST | Reset | 8 | Touch Reset |

### SD Card (Software SPI)

| SD Pin | Signal | Teensy 4.0 Pin | Notes |
|--------|--------|----------------|-------|
| CS | Chip Select | 7 | Software SPI CS |
| MOSI | Data Out | 1 | Software SPI MOSI |
| MISO | Data In | 2 | Software SPI MISO |
| SCK | Clock | 0 | Software SPI SCK (4MHz) |
| VCC | Power | 3.3V | Power |
| GND | Ground | GND | Ground |

### ST7262 Pin Summary (Teensy 4.0)

**Complete 24-bit RGB configuration:**

```
Display Control (Top side):
  HSYNC=4, VSYNC=5, DE=6, BL=23
  Bottom pads: PCLK=32, STBY=33

RGB Data Lines (24 pins):
  Red (Top): R0-R3 [19,18,14,15]
  Red (Bottom): R4-R7 [28,29,30,31]
  
  Green (Top): G0-G3 [22,23,20,21]
  Green (Bottom): G4-G7 [24,25,26,27]
  
  Blue (Top): B0-B5 [10,12,11,13,16,17]
  Blue (Bottom): B6-B7 [32,33]

Touch Controller (Bottom pads):
  I2C1: SDA=25, SCL=24, RST=8 (no INT pin - conflicts with SD)

SD Card (Software SPI on top pins):
  CS=7, MOSI=1, MISO=2, SCK=0
```

**Critical Notes:**
- Pins 32 and 33 are shared between B6/B7 and PCLK/STBY (hardware limitation)
- Requires 21.7V @ 60mA boost converter for backlight
- Not suitable for breadboard prototyping - designed for PCB integration
- Bottom pads required: 24-33 (10 pins total)

---

# Configuration Comparison

| Feature | ST7789 (SPI) | ST7789 (Parallel) | ST7262 (Parallel RGB) |
|---------|-------------|-------------------|----------------------|
| **Resolution** | 240x320 | 240x320 | 800x480 |
| **Size** | 2.4" | 2.4" | 5" |
| **Interface** | 4-wire SPI | 8/16-bit Parallel | 24-bit Parallel RGB |
| **Color Depth** | RGB565 (16-bit) | RGB565 (16-bit) | RGB888 (24-bit) |
| **Data Pins** | 1 pin (MOSI) | 8 or 16 pins | 24 pins |
| **Control Pins** | 4 pins | 4 pins | 5 pins |
| **Touch I2C** | I2C0 (18/19) | I2C1 (24/25)* | I2C1 (24/25) |
| **SD Card** | Hardware SPI1 | Hardware SPI1** | Software SPI |
| **Speed** | Up to 30MHz SPI | Parallel ~10MHz | 30MHz pixel clock |
| **Total Pins** | ~15 pins | ~20 pins (8-bit) | ~37 pins |
| **Backlight** | 3.3V compatible | 3.3V compatible | 21.7V boost needed |

\* Touch conflicts with parallel data pins if using I2C0  
\** 16-bit mode conflicts with SD SPI1 pins

---

# Notes

## ST7789 Notes
1. **Interface Mode:** IM0 and IM2 now GPIO-controlled (pins 4 and 3) - no manual jumper changes needed!
2. **Auto-Configuration:** Driver sets IM[3:0]=1101b for 4-wire SPI automatically
3. **SPI Testing:** Code includes exhaustive SPI speed/mode testing (100kHz-60MHz, MODE0-3, 3-wire/4-wire/bit-bang)
4. **SD Card Options:** 
   - Hardware SPI1 (pins 0,1,26,27) - **Requires bottom pad soldering for pins 26/27**
   - Software SPI alternative available - modify SD_* pin definitions in main.cpp to use top-side pins
5. **Touch Reset:** Shared with display reset on pin 8
6. **Backlight:** 3.3V compatible (no boost converter needed)
7. **Breadboard Friendly:** Main display uses only top-side pins (0-23)

## ST7789 Parallel Notes
1. **Interface Mode:** IM0 and IM2 now GPIO-controlled (pins 4 and 3) - mode switches automatically!
2. **Auto-Configuration:** Driver sets IM[3:0]=0100b for 8-bit (DB8-DB15) or IM[3:0]=0000b for 16-bit (DB0-DB15)
3. **Pin Conflicts:** In 8-bit mode, DB0/DB1 conflict with I2C0 SDA/SCL - use I2C1 for touch
4. **16-bit Mode Conflicts:** DB14/DB15 conflict with SPI1 MOSI/SCK - use software SPI for SD card
5. **Speed:** Faster than SPI mode, but slower than ST7262 dedicated RGB interface
6. **Backlight:** 3.3V compatible (no boost converter needed)
7. **Recommended:** Use 8-bit mode to minimize pin conflicts

## ST7262 Notes
1. **Backlight:** Requires 21.7V @ 60mA boost converter (not 3.3V directly)
2. **Pixel Clock:** 30MHz for 800x480 @ 60Hz refresh
3. **SD Card:** Uses software SPI (CS=3, MOSI=1, MISO=2, SCK=0)
4. **Standby:** STBYB pin 33 (bottom pad) must be HIGH for normal operation
5. **Touch I2C:** Uses I2C1 on bottom solder pads (pins 24/25), no INT pin available
6. **Bottom Pads Required:** R4-R7 (28-31), G4-G7 (24-27), B6-B7 (32-33), PCLK (32), STBY (33)
7. **Not Breadboard Friendly:** Requires extensive bottom pad soldering - designed for PCB use only
8. **No Pin Conflicts:** HSYNC/VSYNC/DE use pins 4/5/6 which don't conflict with ST7789 modes

## General Notes
- All configurations share the same codebase with compile-time selection
- Touch controller (FT5x26) works on all configurations
- SD card works on all (hardware SPI vs software SPI)
- Switch between displays by changing build flag in platformio.ini

## Teensy 4.0 Pin Accessibility

### Top Side Pins (0-23)
Standard 0.1" header spacing - perfect for:
- Breadboard prototyping
- Standard dupont connectors
- Easy hand-soldering

### Bottom Solder Pads (24-33)
Small castellated pads requiring:
- Fine-pitch soldering skills (0.05" spacing)
- Magnification for accurate work
- Wire-wrapping or careful point-to-point wiring
- **Cannot be used with standard breadboards**

### Pin Usage Strategy by Configuration

| Configuration | Top Pins Used | Bottom Pads Required | Breadboard Friendly? |
|--------------|---------------|---------------------|---------------------|
| **ST7789 SPI** | 15 pins | 0-2 pins (SD only) | ✅ **YES** |
| **ST7789 8-bit Parallel** | 12 pins | 8 pins | ⚠️ Moderate |
| **ST7789 16-bit Parallel** | 12 pins | 16 pins | ❌ No |
| **ST7262 RGB** | 14 pins | 10 pins | ❌ **No - PCB only** |

### Recommendations

**For Breadboard Prototyping:**
- Use **ST7789 SPI mode** - requires only 0-2 bottom pad connections
- If using software SPI for SD card: **Zero bottom pad soldering needed!**
- If using hardware SPI1 for SD: Only 2 bottom pads (26, 27)

**For Production/PCB:**
- ST7789 Parallel 8-bit: Good balance of speed and pin count
- ST7262 RGB: Maximum performance for large displays

**Bottom Pad Soldering Tips:**
- Use flux and fine-tip soldering iron
- Solder thin wire-wrap wire (30 AWG)
- Test continuity before connecting peripherals
- Consider hot air rework station for best results
