/**
 * @file test_display_simple.cpp
 * Simple display test - draws color bars to verify parallel interface
 */

#include <Arduino.h>
#include "display.h"

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    
    Serial.println("\n========================================");
    Serial.println("ST7789 Parallel Display Test");
    Serial.println("========================================\n");

    // Initialize display - 16-bit 8080-II mode uses D[17:10], D[8:1] (skips D[9] and D[0])
    // Mapping: D1-D8 to our DB0-DB7, D10-D17 to our DB8-DB15
    display_config_t display_config = {
        .pin_data = {19, 18, 14, 15, 28, 29, 30, 31, 22, 23, 20, 21, 24, 25, 26, 27},  // DB0-DB15
        .pin_dc = 9,
        .pin_cs = 10,
        .pin_wr = 13,
        .pin_rd = 5,
        .pin_rst = 8,
        .pin_bl = 6,
        .pin_im0 = 4,   // IM0 = LOW
        .pin_im2 = 3,   // IM2 = LOW  
        .use_16bit = true  // 16-bit 8080-II mode: IM3=HIGH (hardwired), IM2=LOW, IM1=LOW, IM0=LOW
    };
    
    Serial.println("[TEST] Initializing display...");
    if (!display_init(&display_config)) {
        Serial.println("[TEST] ERROR: Display initialization failed!");
        while(1) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
    
    Serial.println("[TEST] Display initialized successfully");
    Serial.println("[TEST] Turning on backlight...");
    display_on();
    
    delay(500);
    
    // Test 1: Fill screen with red
    Serial.println("[TEST] Test 1: Filling screen RED...");
    display_fill_screen(DISPLAY_RED);
    delay(2000);
    
    // Test 2: Fill screen with green
    Serial.println("[TEST] Test 2: Filling screen GREEN...");
    display_fill_screen(DISPLAY_GREEN);
    delay(2000);
    
    // Test 3: Fill screen with blue
    Serial.println("[TEST] Test 3: Filling screen BLUE...");
    display_fill_screen(DISPLAY_BLUE);
    delay(2000);
    
    // Test 4: Fill screen with white
    Serial.println("[TEST] Test 4: Filling screen WHITE...");
    display_fill_screen(DISPLAY_WHITE);
    delay(2000);
    
    // Test 5: Color bars
    Serial.println("[TEST] Test 5: Drawing color bars...");
    display_fill_screen(DISPLAY_BLACK);
    
    // Draw horizontal color bars using fill_rect
    for (int y = 0; y < 320; y += 40) {
        uint16_t color;
        switch((y / 40) % 8) {
            case 0: color = DISPLAY_RED; break;
            case 1: color = DISPLAY_GREEN; break;
            case 2: color = DISPLAY_BLUE; break;
            case 3: color = DISPLAY_YELLOW; break;
            case 4: color = DISPLAY_CYAN; break;
            case 5: color = DISPLAY_MAGENTA; break;
            case 6: color = DISPLAY_WHITE; break;
            case 7: color = DISPLAY_BLACK; break;
        }
        
        display_fill_rect(0, y, 240, 40, color);
        Serial.print(".");
    }
    Serial.println(" Done!");
    
    Serial.println("\n[TEST] All tests complete!");
    Serial.println("[TEST] You should see 8 horizontal color bars");
}

void loop() {
    // Blink LED to show we're alive
    static uint32_t last_blink = 0;
    if (millis() - last_blink > 1000) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        last_blink = millis();
    }
}
