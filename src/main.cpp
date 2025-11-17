#include <Arduino.h>
#include <SdFat.h>
#include <SPI.h>
#include <Wire.h>
#include "st7789.h"
#include "ft6236.h"

// Pin configuration - ER-TFTM024-3 to Teensy 4.0 (3-wire SPI + I2C Touch)
// 3-Wire SPI Mode: IM0=1, IM2=1

// TFT LCD (ST7789VI) - 3-Wire SPI
#define TFT_DC      9   // Pin 9  → Pin 11 DCX (Data/Command Select)
#define TFT_CS      10  // Pin 10 → Pin 10 CSX (Chip Select)
#define TFT_MOSI    11  // Pin 11 → Pin 9 SDA (MOSI - Serial Data In)
#define TFT_SCLK    13  // Pin 13 → Pin 12 WRX (SCK - SPI Clock)
#define TFT_MISO    12  // Pin 12 → Pin 6 SDO (MISO - for read)
#define TFT_RST     8   // Pin 8  → Pin 30 RESX (Reset, optional)
#define TFT_BL      6   // Pin 6  → Pin 38 LED-A (Backlight PWM)
#define TFT_TE      2   // Pin 2  → Pin 40 TE (Tearing Effect vsync)

// CTP Touch (FT5x26) - I2C
#define TOUCH_SDA   18  // Pin 18 (SDA0) → CTP Pin 4 SDA (I2C Data)
#define TOUCH_SCL   19  // Pin 19 (SCL0) → CTP Pin 3 SCL (I2C Clock)
#define TOUCH_INT   7   // Pin 7  → CTP Pin 5 /INT (Touch Interrupt, optional)

// SD Card - Dedicated SPI1 bus (not shared with display)
#define SD_CS       0   // Pin 0 → SD Card Chip Select
#define SD_MOSI     26  // Pin 26 → SD MOSI (SPI1)
#define SD_MISO     1   // Pin 1 → SD MISO (SPI1)
#define SD_SCK      27  // Pin 27 → SD SCK (SPI1)

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
    Serial.println("Teensy 4.0 ST7789VI + FT5x26 Touch Demo");
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
    
    // Initialize display FIRST (3-wire SPI mode)
    st7789_config_t display_config = {
        .pin_mosi = TFT_MOSI,      // Pin 11 â†’ SDA (Pin 9)
        .pin_miso = TFT_MISO,      // Pin 12 â†’ SDO (Pin 6, optional)
        .pin_sclk = TFT_SCLK,      // Pin 13 â†’ WRX/SCK (Pin 12)
        .pin_cs = TFT_CS,          // Pin 10 â†’ CSX (Pin 10)
        .pin_dc = TFT_DC,          // Pin 9 â†’ DCX (Pin 11)
        .pin_rst = TFT_RST,        // Pin 8 â†’ RESX (Pin 30)
        .pin_bl = TFT_BL,          // Pin 6 â†’ LED-A (Pin 38)
        .spi_clock_mhz = 30        // 30MHz SPI clock for Teensy 4.0
    };
    
    Serial.println("Initializing display...");
    if (!st7789_init(&display_config)) {
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
    st7789_fill_screen(ST7789_WHITE);
    
    // Turn on display AFTER first pixel write (like Newhaven sample)
    Serial.println("Turning on display (sending command 0x29)...");
    st7789_display_on();
    delay(2000);
    
    Serial.println("\nFilling screen with RED...");
    Serial.println("  (Screen should be completely red now)");
    st7789_fill_screen(ST7789_RED);
    delay(2000);
    
    Serial.println("Filling screen with GREEN...");
    Serial.println("  (Screen should be completely green now)");
    st7789_fill_screen(ST7789_GREEN);  
    delay(2000);
    
    Serial.println("Filling screen with BLUE...");
    Serial.println("  (Screen should be completely blue now)");
    st7789_fill_screen(ST7789_BLUE);
    delay(2000);
    
    Serial.println("Filling screen with BLACK...");
    Serial.println("  (Screen should be completely black now)");
    st7789_fill_screen(ST7789_BLACK);
    delay(1000);
    
    // Draw test pattern
    Serial.println("Drawing checkerboard pattern...");
    Serial.println("  (Screen should show black and white squares)");
    for(int y = 0; y < ST7789_HEIGHT; y += 40) {
        for(int x = 0; x < ST7789_WIDTH; x += 40) {
            uint16_t color = ((x/40) + (y/40)) % 2 ? ST7789_WHITE : ST7789_BLACK;
            st7789_fill_rect(x, y, 40, 40, color);
        }
    }
    
    Serial.println("\n*** DISPLAY TEST COMPLETE ***");
    Serial.println("If you saw the screen change colors, display is WORKING");
    Serial.println("If screen stayed blank/white, display needs troubleshooting");
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
    Serial.println("Initializing SD card on dedicated SPI1 bus...");
    Serial.printf("SD CS Pin: %d\n", SD_CS);
    Serial.printf("SPI1: MOSI=%d, MISO=%d, SCK=%d\n", SD_MOSI, SD_MISO, SD_SCK);
    
    // Initialize SPI1 bus for SD card
    SPI1.setMOSI(SD_MOSI);
    SPI1.setMISO(SD_MISO);
    SPI1.setSCK(SD_SCK);
    SPI1.begin();
    
    // Initialize SD card on SPI1 with SdFat
    if (!SD.begin(SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(25), &SPI1))) {
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
