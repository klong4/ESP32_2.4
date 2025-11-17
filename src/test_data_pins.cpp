/**
 * @file test_data_pins.cpp
 * Test individual data pins by sending specific patterns
 */

#include <Arduino.h>
#include "display.h"

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    
    Serial.println("\n========================================");
    Serial.println("ST7789 Data Pin Test");
    Serial.println("========================================\n");

    // Initialize display
    display_config_t display_config = {
        .pin_data = {19, 18, 14, 15, 28, 29, 30, 31},  // DB0-DB7
        .pin_dc = 9,
        .pin_cs = 10,
        .pin_wr = 13,
        .pin_rd = 5,
        .pin_rst = 8,
        .pin_bl = 6,
        .pin_im0 = 4,
        .pin_im2 = 3,
        .use_16bit = false
    };
    
    Serial.println("[TEST] Initializing display...");
    if (!display_init(&display_config)) {
        Serial.println("[TEST] ERROR: Display initialization failed!");
        while(1) delay(1000);
    }
    
    Serial.println("[TEST] Display initialized");
    display_on();
    delay(500);
    
    // Test pure colors to diagnose which channels work
    Serial.println("\n[TEST] Testing pure red (0xF800)...");
    Serial.println("  RGB565: 11111 000000 00000");
    Serial.println("  Bytes: 0xF8, 0x00");
    Serial.println("  Should see: PURE RED");
    display_fill_screen(0xF800);  // Pure red
    delay(3000);
    
    Serial.println("\n[TEST] Testing pure green (0x07E0)...");
    Serial.println("  RGB565: 00000 111111 00000");
    Serial.println("  Bytes: 0x07, 0xE0");
    Serial.println("  Should see: PURE GREEN");
    display_fill_screen(0x07E0);  // Pure green
    delay(3000);
    
    Serial.println("\n[TEST] Testing pure blue (0x001F)...");
    Serial.println("  RGB565: 00000 000000 11111");
    Serial.println("  Bytes: 0x00, 0x1F");
    Serial.println("  Should see: PURE BLUE");
    display_fill_screen(0x001F);  // Pure blue
    delay(3000);
    
    Serial.println("\n[TEST] Testing white (0xFFFF)...");
    Serial.println("  RGB565: 11111 111111 11111");
    Serial.println("  Bytes: 0xFF, 0xFF");
    Serial.println("  Should see: WHITE");
    display_fill_screen(0xFFFF);  // White
    delay(3000);
    
    Serial.println("\n[TEST] Testing black (0x0000)...");
    Serial.println("  RGB565: 00000 000000 00000");
    Serial.println("  Bytes: 0x00, 0x00");
    Serial.println("  Should see: BLACK");
    display_fill_screen(0x0000);  // Black
    delay(2000);
    
    Serial.println("\n[TEST] Testing cyan (0x07FF)...");
    Serial.println("  RGB565: 00000 111111 11111 (Green+Blue)");
    Serial.println("  Bytes: 0x07, 0xFF");
    Serial.println("  Should see: CYAN (if you see green only, blue channel dead)");
    display_fill_screen(0x07FF);  // Cyan
    delay(3000);
    
    Serial.println("\n[TEST] Testing magenta (0xF81F)...");
    Serial.println("  RGB565: 11111 000000 11111 (Red+Blue)");
    Serial.println("  Bytes: 0xF8, 0x1F");
    Serial.println("  Should see: MAGENTA (if you see red only, blue channel dead)");
    display_fill_screen(0xF81F);  // Magenta
    delay(3000);
    
    Serial.println("\n[TEST] Testing yellow (0xFFE0)...");
    Serial.println("  RGB565: 11111 111111 00000 (Red+Green)");
    Serial.println("  Bytes: 0xFF, 0xE0");
    Serial.println("  Should see: YELLOW");
    display_fill_screen(0xFFE0);  // Yellow
    delay(3000);
    
    Serial.println("\n========================================");
    Serial.println("DIAGNOSTIC COMPLETE");
    Serial.println("========================================");
    Serial.println("\nREPORT what you see:");
    Serial.println("1. Did you see RED?");
    Serial.println("2. Did you see GREEN?");
    Serial.println("3. Did you see BLUE?");
    Serial.println("4. Did CYAN look like green or proper cyan?");
    Serial.println("5. Did MAGENTA look like red or proper magenta?");
    Serial.println("\nIf BLUE doesn't show:");
    Serial.println("  - Check DB0-DB4 wiring (pins 19,18,14,15,28)");
    Serial.println("  - Blue uses bits 0-4 in RGB565");
}

void loop() {
    // Blink LED
    static uint32_t last_blink = 0;
    if (millis() - last_blink > 1000) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        last_blink = millis();
    }
}
