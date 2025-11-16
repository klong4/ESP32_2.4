# Teensy 4.0 Display Configuration

## Hardware
- **Display**: ST7789VI (240x320)
- **Touch**: FT5x26 Capacitive Touch (I2C)
- **Interface**: 3-wire SPI (IM0=1, IM2=1)

## Interface Mode Selection
- **IM0** (Pin 31) = **3.3V (HIGH)** - 3-wire SPI mode
- **IM2** (Pin 32) = **3.3V (HIGH)** - 3-wire SPI mode

## Pin Connections

### TFT Display (ST7789VI) - 3-Wire SPI

| Display Pin | Pin Name | Function | Teensy 4.0 Pin | Notes |
|------------|----------|----------|----------------|-------|
| 1 | GND | Ground | GND | Power ground |
| **6** | **SDO** | **MISO** | **12** | **Serial Data Out - connected for read operations** |
| 7 | VDD | Power (3.3V) | 3.3V | Display power supply |
| 8 | VDDI | Logic (3.3V) | 3.3V | Logic level supply |
| **9** | **SDA** | **MOSI** | **11** | **Serial Data In** |
| **10** | **CSX** | **CS** | **10** | **Chip Select (Active Low)** |
| **11** | **DCX** | **D/C** | **9** | **Data/Command Select** |
| **12** | **WRX** | **SCK** | **13** | **SPI Clock (Write Clock)** |
| 13 | RDX | Read Enable | 3.3V | Tie HIGH (read not used) |
| 14-29 | DB0-DB15 | Parallel Data | NC | Not used in SPI mode |
| 30 | RESX | Reset | 8 | Active Low Reset (optional) |
| **31** | **IM0** | Mode Select | **3.3V** | **Set HIGH for 3-wire SPI** |
| **32** | **IM2** | Mode Select | **3.3V** | **Set HIGH for 3-wire SPI** |
| 33 | GND | Ground | GND | Ground |
| 34-37 | LED-K1-4 | Backlight Cathode | GND | Connect all to ground |
| **38** | **LED-A** | **Backlight Anode** | **6** | **PWM control via resistor** |
| 39 | GND | Ground | GND | Ground |
| **40** | **TE** | **Tearing Effect** | **2** | **Optional vsync signal, connected to Pin 2** |

### Capacitive Touch Panel (FT5x26) - I2C

| CTP Pin | Pin Name | Function | Teensy 4.0 Pin | Notes |
|---------|----------|----------|----------------|-------|
| 1 | VDD | Power (3.3V) | 3.3V | Touch controller power |
| 2 | VSS | Ground | GND | Ground |
| **3** | **SCL** | **I2C Clock** | **19 (SCL0)** | **Requires 4.7kΩ pull-up** |
| **4** | **SDA** | **I2C Data** | **18 (SDA0)** | **Requires 4.7kΩ pull-up** |
| 5 | /INT | Interrupt | 7 | Touch interrupt (optional) |
| 6 | /RESET | Reset | 8 | Active Low Reset (can share with display) |

### SD Card - SPI1 (Secondary SPI Bus)

| SD Card Pin | Function | Teensy 4.0 Pin | Notes |
|-------------|----------|----------------|-------|
| **CS** | **Chip Select** | **1** | **Any free GPIO, using Pin 1** |
| **MOSI** | **SPI MOSI** | **11** | **Shared with display MOSI** |
| **MISO** | **SPI MISO** | **12** | **Shared with display MISO** |
| **SCK** | **SPI Clock** | **13** | **Shared with display SCK** |
| VCC | Power (3.3V) | 3.3V | SD card power |
| GND | Ground | GND | Ground |

**Note**: SD card shares the same SPI bus as the display. Devices are selected using different CS pins (Pin 10 for display, Pin 1 for SD card).

## Teensy 4.0 Pin Summary

| Teensy Pin | Function | Connected To |
|------------|----------|--------------|
| 3.3V | Power | VDD, VDDI, IM0, IM2, RDX |
| GND | Ground | GND pins, LED-K1-4 |
| **1** | **SD Card CS** | **SD CS** |
| **2** | **Tearing Effect** | **TE (Pin 40)** |
| 6 | Backlight PWM | LED-A via resistor |
| 7 | Touch Interrupt | CTP /INT |
| 8 | Reset | RESX & CTP /RESET |
| **9** | **D/C Select** | **DCX (Pin 11)** |
| **10** | **Display CS** | **CSX (Pin 10)** |
| **11** | **SPI MOSI** | **Display SDA (Pin 9) + SD MOSI** |
| **12** | **SPI MISO** | **Display SDO (Pin 6) + SD MISO** |
| **13** | **SPI SCK** | **Display WRX (Pin 12) + SD SCK** |
| 18 | I2C SDA | CTP SDA |
| 19 | I2C SCL | CTP SCL |

## 3-Wire SPI Protocol

In 3-wire SPI mode on the ST7789VI:
- **WRX (Pin 12)** acts as the **SPI Clock** (SCK)
- **DCX (Pin 11)** acts as **Data/Command** select (NOT clock)
  - LOW = Command mode
  - HIGH = Data mode
- **SDA (Pin 9)** is the **MOSI** line for serial data
- **CSX (Pin 10)** is the **Chip Select** (Active Low)
- **SDO (Pin 6)** is **MISO** for read operations (connected to Pin 12)
- **TE (Pin 40)** provides **Tearing Effect** signal for vsync (connected to Pin 2)

## Important Notes

1. **Interface Mode Jumpers**:
   - Both IM0 and IM2 must be tied to 3.3V (HIGH) for 3-wire SPI
   - RDX should be tied HIGH since we're not using read operations

2. **Backlight**:
   - LED-A (Pin 38) requires a current-limiting resistor (100-150Ω for ~20mA)
   - Connect to Teensy Pin 6 for PWM brightness control
   - LED-K1-4 all connect to GND

3. **I2C Pull-ups**:
   - SCL and SDA lines require 4.7kΩ pull-up resistors to 3.3V
   - Teensy 4.0 has internal pull-ups but external 4.7kΩ recommended

4. **Power**:
   - All power connections use 3.3V (NOT 5V)
   - Teensy 4.0 provides 3.3V from its voltage regulator

5. **SPI Speed**:
   - Display (SPI): Configured for 30MHz on Teensy 4.0 (can be increased up to 50MHz)
   - SD Card (SPI): Typically 25MHz or lower for reliability

6. **Shared SPI Bus**:
   - Display and SD card share the same SPI bus (Pins 11, 12, 13)
   - Devices are selected using different CS pins:
     - **Display CS**: Pin 10
     - **SD Card CS**: Pin 1
   - Only one device can be active at a time (CS LOW)

7. **Shared Reset Pin**:
   - Pin 8 can be safely shared between display (RESX) and touch (CTP /RESET)
   - SD cards don't use a reset pin, only CS for device selection

## Software Configuration

```cpp
// Pin definitions
#define TFT_DC      9   // Data/Command Select
#define TFT_CS      10  // Chip Select
#define TFT_MOSI    11  // SPI MOSI
#define TFT_SCLK    13  // SPI Clock
#define TFT_MISO    12  // SPI MISO
#define TFT_RST     8   // Reset
#define TFT_BL      6   // Backlight PWM
#define TFT_TE      2   // Tearing Effect (vsync)

#define TOUCH_SDA   18  // I2C Data
#define TOUCH_SCL   19  // I2C Clock
#define TOUCH_INT   7   // Touch Interrupt

#define SD_CS       1   // SD Card Chip Select
// SD shares SPI bus with display (pins 11, 12, 13)

// Display configuration
ili9341_config_t display_config = {
    .pin_mosi = TFT_MOSI,
    .pin_miso = TFT_MISO,
    .pin_sclk = TFT_SCLK,
    .pin_cs = TFT_CS,
    .pin_dc = TFT_DC,
    .pin_rst = TFT_RST,
    .pin_bl = TFT_BL,
    .spi_clock_mhz = 30
};

// SD Card uses same SPI bus, different CS pin
// Initialize with: SD.begin(SD_CS)
```

## Testing

1. Verify all power connections (3.3V and GND)
2. Check IM0 and IM2 are both at 3.3V
3. Upload test firmware
4. Display should show RGB color sequence (Blue → Green → Red)
5. Touch should be responsive on I2C

## Troubleshooting

- **No display**: Check IM0, IM2 at 3.3V and all SPI connections
- **White screen**: Check DC pin and verify it's connected to DCX (Pin 11)
- **Garbled display**: Reduce SPI clock speed or check clock connection
- **No backlight**: Check LED-A resistor and PWM pin connection
- **No touch**: Verify I2C pull-ups and check address (0x38)
