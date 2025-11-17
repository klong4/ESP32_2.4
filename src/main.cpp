#include <Arduino.h>
#include <SdFat.h>
#include <SPI.h>
#include <Wire.h>
#include "display.h"
#include "ft6236.h"

// Display selection info
#ifdef DISPLAY_ST7789
    #define DISPLAY_NAME "ST7789VI 240x320 (SPI)"
#elif defined(DISPLAY_ST7789_PARALLEL)
    #define DISPLAY_NAME "ST7789VI 240x320 (8/16-bit Parallel)"
#elif defined(DISPLAY_ST7262)
    #define DISPLAY_NAME "ST7262 800x480 (24-bit Parallel RGB)"
#endif

// Pin configuration - ER-TFTM024-3 to Teensy 4.0 (3-wire SPI + I2C Touch)
// 3-Wire SPI Mode: IM0=1, IM2=1
// 
// NOTE: Teensy 4.0 has pins 0-23 on top (easy access) and pins 24-33 as bottom solder pads
// This configuration minimizes bottom pad usage for breadboard-friendly prototyping

// TFT LCD (ST7789VI) - 3-Wire SPI
#define TFT_DC      9   // Pin 9  → Pin 11 DCX (Data/Command Select)
#define TFT_CS      10  // Pin 10 → Pin 10 CSX (Chip Select)
#define TFT_MOSI    11  // Pin 11 → Pin 9 SDA (MOSI - Serial Data In)
#define TFT_SCLK    13  // Pin 13 → Pin 12 WRX (SCK - SPI Clock)
#define TFT_MISO    12  // Pin 12 → Pin 6 SDO (MISO - for read)
#define TFT_RST     8   // Pin 8  → Pin 30 RESX (Reset, optional)
#define TFT_BL      6   // Pin 6  → Pin 38 LED-A (Backlight PWM)
#define TFT_TE      2   // Pin 2  → Pin 40 TE (Tearing Effect vsync)
#define TFT_IM0     4   // Pin 4  → Pin 31 IM0 (Interface Mode 0)
#define TFT_IM2     3   // Pin 3  → Pin 32 IM2 (Interface Mode 2)

// CTP Touch (FT5x26)
#ifdef DISPLAY_ST7789
// I2C0 (Hardware I2C) for ST7789 SPI mode
#define TOUCH_SDA   18  // Pin 18 (SDA0) → CTP Pin 4 SDA (I2C Data)
#define TOUCH_SCL   19  // Pin 19 (SCL0) → CTP Pin 3 SCL (I2C Clock)
#define TOUCH_INT   17  // Pin 17  → CTP Pin 5 /INT (Touch Interrupt, optional)
#define TOUCH_RST   8   // Pin 8 → CTP Pin 6 /RST (Touch Reset, shared with display)
#else
// Software I2C for ST7789 Parallel and ST7262 (to avoid pin conflicts)
#define TOUCH_SDA   17  // Pin 17 → CTP Pin 4 SDA (Software I2C Data)
#define TOUCH_SCL   16  // Pin 16 → CTP Pin 3 SCL (Software I2C Clock)
#define TOUCH_INT   32  // Pin 32 → CTP Pin 5 /INT (Touch Interrupt, bottom pad)
#define TOUCH_RST   8   // Pin 8 → CTP Pin 6 /RST (Touch Reset)
#endif

// SD Card - Software SPI
#ifdef DISPLAY_ST7789
// For ST7789 SPI mode - Hardware SPI1
#define SD_CS       0   // Pin 0 → SD Card Chip Select
#define SD_MOSI     26  // Pin 26 → SD MOSI (Hardware SPI1)
#define SD_MISO     1   // Pin 1 → SD MISO (Hardware SPI1)
#define SD_SCK      27  // Pin 27 → SD SCK (Hardware SPI1)
#elif defined(DISPLAY_ST7789_PARALLEL)
// For ST7789 Parallel mode - Software SPI (to avoid conflicts with DB14/DB15)
#define SD_CS       0   // Pin 0 → SD Card Chip Select
#define SD_MOSI     1   // Pin 1 → SD MOSI (Software SPI)
#define SD_MISO     2   // Pin 2 → SD MISO (Software SPI)
#define SD_SCK      7   // Pin 7 → SD SCK (Software SPI)
#else
// For ST7262 mode - Software SPI
#define SD_CS       7   // Pin 7 → SD Card Chip Select
#define SD_MOSI     1   // Pin 1 → SD MOSI (Software SPI)
#define SD_MISO     2   // Pin 2 → SD MISO (Software SPI)
#define SD_SCK      0   // Pin 0 → SD SCK (Software SPI)
#endif

// NOTE: Interface Mode Selection for 4-Wire SPI
// IM0 (Pin 31) = 3.3V (HIGH)
// IM1 (internal) = Pulled LOW
// IM2 (Pin 32) = 3.3V (HIGH)
// IM3 (internal) = Pulled LOW
// Result: IM[3:0] = 1101b = 4-wire SPI with DC pin

void test_touch();
void test_sd_card();

// SD card object using SPI1
SdFat SD;
SdFile testFile;

void setup() {
    // Wait 3 seconds for serial monitor to connect
    Serial.begin(115200);
    delay(3000);
    
    Serial.println("");
    Serial.println("========================================");
    Serial.print("Teensy 4.0 Display Test - ");
    Serial.println(DISPLAY_NAME);
    Serial.println("========================================");
    Serial.println("");
    
    // Test backlight first
    Serial.println("Testing backlight...");
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    Serial.println("Backlight should be ON now");
    
    // Test the SPI pins manually before init
    Serial.println("\nTesting SPI pins manually...");
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_MOSI, OUTPUT);
    pinMode(TFT_SCLK, OUTPUT);
    
    digitalWrite(TFT_CS, LOW);
    digitalWrite(TFT_DC, LOW); // Command
    
    // Manually send a byte (0xAA = 10101010)
    Serial.println("Sending test pattern 0xAA on MOSI...");
    for(int i = 0; i < 8; i++) {
        digitalWrite(TFT_SCLK, LOW);
        digitalWrite(TFT_MOSI, (i % 2) ? HIGH : LOW); // Alternating pattern
        delayMicroseconds(10);
        digitalWrite(TFT_SCLK, HIGH);
        delayMicroseconds(10);
    }
    
    digitalWrite(TFT_CS, HIGH);
    Serial.println("Manual SPI test complete - check MOSI/SCK with scope");
    delay(2000);
    
    // Initialize display FIRST
    #ifdef DISPLAY_ST7789
    st7789_config_t display_config = {
        .pin_mosi = TFT_MOSI,      // Pin 11 → SDA (Pin 9)
        .pin_miso = TFT_MISO,      // Pin 12 → SDO (Pin 6, optional)
        .pin_sclk = TFT_SCLK,      // Pin 13 → WRX/SCK (Pin 12)
        .pin_cs = TFT_CS,          // Pin 10 → CSX (Pin 10)
        .pin_dc = TFT_DC,          // Pin 9 → DCX (Pin 11)
        .pin_rst = TFT_RST,        // Pin 8 → RESX (Pin 30)
        .pin_bl = TFT_BL,          // Pin 6 → LED-A (Pin 38)
        .pin_im0 = TFT_IM0,        // Pin 4 → IM0 (Pin 31)
        .pin_im2 = TFT_IM2,        // Pin 3 → IM2 (Pin 32)
        .spi_clock_mhz = 30        // 30MHz SPI clock for Teensy 4.0
    };
    #elif defined(DISPLAY_ST7789_PARALLEL)
    // ST7789VI 8-bit parallel configuration
    st7789_parallel_config_t display_config = {
        .pin_data = {19, 18, 14, 15, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 0, 0}, // DB0-DB7 (8-bit mode, DB4-7 on bottom)
        .pin_dc = 9,       // Data/Command
        .pin_cs = 10,      // Chip Select
        .pin_wr = 13,      // Write strobe (WRX)
        .pin_rd = 5,       // Read strobe (RDX)
        .pin_rst = 8,      // Reset
        .pin_bl = 6,       // Backlight
        .pin_im0 = 4,      // IM0 - Interface mode bit 0
        .pin_im2 = 3,      // IM2 - Interface mode bit 2
        .use_16bit = false // 8-bit mode (set true for 16-bit)
    };
    #elif defined(DISPLAY_ST7262)
    // ST7262 24-bit parallel RGB configuration
    st7262_config_t display_config = {
        .pin_r = {19, 18, 14, 15, 28, 29, 30, 31}, // R0-R7 (R4-7 on bottom)
        .pin_g = {22, 23, 20, 21, 24, 25, 26, 27}, // G0-G7 (G4-7 on bottom)
        .pin_b = {10, 12, 11, 13, 16, 17, 32, 33}, // B0-B7 (B6-7 on bottom, note: 32/33 shared with PCLK/STBY!)
        .pin_de = 6,       // Data Enable
        .pin_vsync = 5,    // Vertical Sync
        .pin_hsync = 4,    // Horizontal Sync
        .pin_pclk = 32,    // Pixel Clock (bottom, conflicts with B6!)
        .pin_rst = 33,     // Standby (bottom, conflicts with B7!)
        .pin_bl = 23,      // Backlight control (PWM)
        .pixel_clock_hz = 30000000  // 30MHz pixel clock
    };
    #endif
    
    Serial.println("Initializing display...");
    if (!display_init(&display_config)) {
        Serial.println("Display initialization failed!");
        return;
    }
    
    Serial.println("Display initialized successfully");
    Serial.println("Backlight turned on by driver");
    
    Serial.println("\n*** DISPLAY TEST - Watch the screen for color changes ***");
    Serial.println("Expected: Full screen color changes every 2 seconds");
    Serial.println("WHITE -> RED -> GREEN -> BLUE -> BLACK -> Checkerboard");
    Serial.println("");
    
    // Test display with full screen colors
    Serial.println("Filling screen with WHITE...");
    Serial.println("  (Screen should be completely white now)");
    display_fill_screen(DISPLAY_WHITE);
    
    // Turn on display AFTER first pixel write (like Newhaven sample)
    Serial.println("Turning on display (sending command 0x29)...");
    display_on();
    delay(2000);
    
    #ifdef DISPLAY_ST7789
    // Test different SPI speeds, modes, and communication types (ST7789 only)
    Serial.println("\n======================================");
    Serial.println("SPI Speed, Mode, and Type Testing");
    Serial.println("======================================\n");
    
    uint32_t speeds[] = {100000, 250000, 500000, 750000, 1000000, 1500000, 2000000, 3000000, 4000000, 
                         6000000, 8000000, 10000000, 12000000, 16000000, 20000000, 24000000, 30000000, 
                         36000000, 40000000, 48000000, 54000000, 60000000};
    const char* speed_names[] = {"100kHz", "250kHz", "500kHz", "750kHz", "1MHz", "1.5MHz", "2MHz", "3MHz", "4MHz",
                                 "6MHz", "8MHz", "10MHz", "12MHz", "16MHz", "20MHz", "24MHz", "30MHz",
                                 "36MHz", "40MHz", "48MHz", "54MHz", "60MHz"};
    uint8_t modes[] = {SPI_MODE0, SPI_MODE1, SPI_MODE2, SPI_MODE3};
    const char* mode_names[] = {"MODE0", "MODE1", "MODE2", "MODE3"};
    uint8_t spi_types[] = {ST7789_SPI_MODE_HW_4WIRE, ST7789_SPI_MODE_HW_3WIRE, ST7789_SPI_MODE_BITBANG};
    const char* spi_type_names[] = {"4-Wire HW", "3-Wire HW", "Bit-Bang"};
    
    for (int t = 0; t < 3; t++) {
        Serial.print("\n========== Testing ");
        Serial.print(spi_type_names[t]);
        Serial.println(" SPI ==========");
        
        for (int m = 0; m < 4; m++) {
            Serial.print("\n*** ");
            Serial.print(spi_type_names[t]);
            Serial.print(" - ");
            Serial.print(mode_names[m]);
            Serial.println(" ***\n");
            
            for (int s = 0; s < 22; s++) {
                Serial.print("Testing ");
                Serial.print(speed_names[s]);
                Serial.print(" @ ");
                Serial.print(spi_type_names[t]);
                Serial.print(" ");
                Serial.print(mode_names[m]);
                Serial.print("... ");
                
                // Set SPI mode and speed
                st7789_set_spi_mode(spi_types[t]);
                st7789_set_spi_settings(speeds[s], modes[m]);
                
                // Full re-initialization for each test
                if (!display_init(&display_config)) {
                    Serial.println("Init failed!");
                    continue;
                }
                display_on();
                
                // Draw test pattern: RED top half, BLUE bottom half
                display_fill_rect(0, 0, 240, 160, DISPLAY_RED);
                display_fill_rect(0, 160, 240, 160, DISPLAY_BLUE);
                
                Serial.println("Done.");
                delay(800); // Give time to observe
            }
        }
    }
    
    Serial.println("\n======================================");
    Serial.println("Speed/Mode testing complete!");
    Serial.println("======================================\n");
    
    // Final test with checkerboard at best guess settings
    Serial.println("Final test: Checkerboard at 4MHz MODE0...");
    display_set_spi_settings(4000000, SPI_MODE0);
    for (int y = 0; y < 320; y += 20) {
        for (int x = 0; x < 240; x += 20) {
            display_color_t color = ((x / 20 + y / 20) % 2) ? DISPLAY_WHITE : DISPLAY_BLACK;
            display_fill_rect(x, y, 20, 20, color);
        }
    }
    Serial.println("Display test complete!");
    #else
    // Simple color test for parallel displays
    Serial.println("\nTesting parallel RGB display...");
    Serial.println("RED screen...");
    display_fill_screen(DISPLAY_RED);
    delay(2000);
    Serial.println("GREEN screen...");
    display_fill_screen(DISPLAY_GREEN);
    delay(2000);
    Serial.println("BLUE screen...");
    display_fill_screen(DISPLAY_BLUE);
    delay(2000);
    Serial.println("Display test complete!");
    #endif
    
    Serial.println("\n*** DISPLAY TEST COMPLETE ***");
    Serial.println("Check serial output to see which speed/mode worked");
    Serial.println("");
    
    // Test touch controller
    Serial.println("\n========================================");
    Serial.println("Testing FT5x26 Touch Controller");
    Serial.println("========================================");
    test_touch();
    
    // Test SD card
    Serial.println("\n========================================");
    Serial.println("Testing SD Card");
    Serial.println("========================================");
    test_sd_card();
    
    Serial.println("\n========================================");
    Serial.println("All tests complete!");
    Serial.println("========================================");
}

void loop() {
    // Touch monitoring disabled - enable if needed
    // Uncomment below to see live touch coordinates
    /*
    static unsigned long last_touch_check = 0;
    
    if (millis() - last_touch_check > 100) { // Check every 100ms
        last_touch_check = millis();
        
        ft5x26_touch_t touch_data;
        if (ft5x26_read_touch(&touch_data)) {
            for (int i = 0; i < touch_data.touch_count; i++) {
                Serial.printf("Touch %d: X=%d, Y=%d, Event=%d\n", 
                    i, 
                    touch_data.points[i].x, 
                    touch_data.points[i].y,
                    touch_data.points[i].event);
                
                // Draw a small circle at touch point
                st7789_fill_rect(touch_data.points[i].x - 5, 
                                touch_data.points[i].y - 5, 
                                10, 10, ST7789_RED);
            }
        }
    }
    */
    
    delay(100);
}

void test_touch() {
    ft5x26_config_t touch_config = {
        .pin_sda = TOUCH_SDA,
        .pin_scl = TOUCH_SCL,
        .pin_int = TOUCH_INT,
        .pin_rst = TFT_RST  // Shared reset with display
    };
    
    Serial.println("Initializing FT5x26 touch controller...");
    if (!ft5x26_init(&touch_config)) {
        Serial.println("ERROR: Touch initialization failed!");
        Serial.println("Check I2C connections (SDA=18, SCL=19)");
        Serial.println("Check pull-up resistors (4.7kΩ recommended)");
        return;
    }
    
    Serial.println("Touch initialized successfully!");
    Serial.println("Testing I2C scanner to verify address...");
    
    // I2C Scanner
    Wire.begin();
    Wire.setClock(400000);
    
    int deviceCount = 0;
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            deviceCount++;
        }
    }
    
    if (deviceCount == 0) {
        Serial.println("WARNING: No I2C devices found!");
        Serial.println("Check connections and pull-up resistors");
    } else {
        Serial.printf("Found %d I2C device(s)\n", deviceCount);
    }
    
    Serial.println("\nTouch test ready - touch the screen to see coordinates");
    Serial.println("Testing for 5 seconds...");
    
    unsigned long start = millis();
    bool touch_detected = false;
    
    while (millis() - start < 5000) {
        ft5x26_touch_t touch_data;
        if (ft5x26_read_touch(&touch_data)) {
            touch_detected = true;
            for (int i = 0; i < touch_data.touch_count; i++) {
                Serial.printf("Touch %d: X=%d, Y=%d, Event=%d, ID=%d\n", 
                    i, 
                    touch_data.points[i].x, 
                    touch_data.points[i].y,
                    touch_data.points[i].event,
                    touch_data.points[i].id);
            }
        }
        delay(50);
    }
    
    if (!touch_detected) {
        Serial.println("No touch detected during test period");
        Serial.println("This may be normal - touch controller waits for first touch");
    }
}

void test_sd_card() {
    Serial.println("Initializing SD card...");
    Serial.printf("SD CS Pin: %d\n", SD_CS);
    Serial.printf("SPI: MOSI=%d, MISO=%d, SCK=%d\n", SD_MOSI, SD_MISO, SD_SCK);
    
    #ifdef DISPLAY_ST7262
    // Use software SPI for ST7262 (hardware SPI pins used by display)
    Serial.println("Using Software SPI...");
    SoftSpiDriver<SD_MISO, SD_MOSI, SD_SCK> softSpi;
    if (!SD.begin(SdSpiConfig(SD_CS, DEDICATED_SPI, SD_SCK_MHZ(4), &softSpi))) {
    #else
    // Initialize SPI1 bus for SD card
    Serial.println("Using Hardware SPI1...");
    SPI1.setMOSI(SD_MOSI);
    SPI1.setMISO(SD_MISO);
    SPI1.setSCK(SD_SCK);
    SPI1.begin();
    
    // Initialize SD card on SPI1 with SdFat
    if (!SD.begin(SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(25), &SPI1))) {
    #endif
        Serial.println("ERROR: SD card initialization failed!");
        Serial.println("Possible causes:");
        Serial.println("  - No SD card inserted");
        Serial.println("  - Card not formatted (use FAT32)");
        Serial.println("  - Bad connections on SPI1 pins:");
        Serial.printf("    CS=%d, MOSI=%d, MISO=%d, SCK=%d\n", SD_CS, SD_MOSI, SD_MISO, SD_SCK);
        Serial.println("  - Incompatible card");
        return;
    }
    
    Serial.println("SD card initialized successfully!");
    
    // Note: Teensy SD library doesn't have cardType() or detailed size info
    // Just verify we can access the card
    Serial.println("SD card is accessible");
    
    // Get volume info if available
    Serial.println("Testing file system access...");
    
    // Test file operations
    Serial.println("\nTesting file operations...");
    
    if (testFile.open("test.txt", O_WRONLY | O_CREAT | O_TRUNC)) {
        testFile.println("Hello from Teensy 4.0 on SPI1!");
        testFile.printf("Millis: %lu\n", millis());
        testFile.close();
        Serial.println("Test file written successfully");
    } else {
        Serial.println("ERROR: Failed to open test file for writing");
    }
    
    if (testFile.open("test.txt", O_RDONLY)) {
        Serial.println("Test file contents:");
        while (testFile.available()) {
            Serial.write(testFile.read());
        }
        testFile.close();
    } else {
        Serial.println("ERROR: Failed to open test file for reading");
    }
    
    // List root directory
    Serial.println("\nRoot directory contents:");
    SdFile root;
    SdFile file;
    
    if (root.open("/")) {
        while (file.openNext(&root, O_RDONLY)) {
            char name[64];
            file.getName(name, sizeof(name));
            
            if (file.isDirectory()) {
                Serial.printf("  %s/ (DIR)\n", name);
            } else {
                Serial.printf("  %s (%lu bytes)\n", name, file.fileSize());
            }
            file.close();
        }
        root.close();
    }
    
    Serial.println("SD card test complete");
}
