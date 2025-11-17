/**
 * @file test_display_modes.cpp
 * Iterates through all IM0/IM2 combinations to find working mode
 */

#include <Arduino.h>
#include "display.h"

const char* mode_names[] = {
    "Mode 0: IM0=0, IM2=0",
    "Mode 1: IM0=1, IM2=0",
    "Mode 2: IM0=0, IM2=1",
    "Mode 3: IM0=1, IM2=1"
};

void test_mode(uint8_t mode_idx) {
    bool im0_val = (mode_idx & 0x01) ? true : false;
    bool im2_val = (mode_idx & 0x02) ? true : false;
    
    Serial.println("\n\n========================================");
    Serial.printf("TESTING %s\n", mode_names[mode_idx]);
    Serial.printf("IM0=%d, IM2=%d\n", im0_val ? 1 : 0, im2_val ? 1 : 0);
    Serial.println("========================================\n");

    // Set IM pins
    pinMode(4, OUTPUT);
    pinMode(3, OUTPUT);
    digitalWrite(4, im0_val ? HIGH : LOW);
    digitalWrite(3, im2_val ? HIGH : LOW);
    delay(10);
    
    // Initialize display
    display_config_t display_config = {
        .pin_data = {19, 18, 14, 15, 28, 29, 30, 31, 22, 23, 20, 21, 24, 25, 26, 27},
        .pin_dc = 9,
        .pin_cs = 10,
        .pin_wr = 13,
        .pin_rd = 5,
        .pin_rst = 8,
        .pin_bl = 6,
        .pin_im0 = 4,
        .pin_im2 = 3,
        .use_16bit = true
    };
    
    if (!display_init(&display_config)) {
        Serial.println("Init failed!");
        return;
    }
    
    display_on();
    delay(500);
    
    // Draw color bars
    Serial.println("Drawing color bars...");
    uint16_t colors[] = {
        DISPLAY_RED, DISPLAY_GREEN, DISPLAY_BLUE, DISPLAY_YELLOW,
        DISPLAY_CYAN, DISPLAY_MAGENTA, DISPLAY_WHITE, DISPLAY_BLACK
    };
    
    for (int i = 0; i < 8; i++) {
        display_fill_rect(0, i * 40, 240, 40, colors[i]);
    }
    
    Serial.println("Color bars drawn! Waiting 5 seconds...");
    
    // Blink LED
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.println("\n\n");
    Serial.println("==========================================");
    Serial.println("ST7789 MODE ITERATION TEST");
    Serial.println("==========================================");
    Serial.println("Testing all IM0/IM2 combinations...");
    Serial.println("Watch for color bars on the display!");
    Serial.println("==========================================\n");
}

void loop() {
    for (uint8_t mode = 0; mode < 4; mode++) {
        test_mode(mode);
    }
    
    Serial.println("\n\n*** CYCLE COMPLETE - Restarting in 3 seconds ***\n\n");
    delay(3000);
}
