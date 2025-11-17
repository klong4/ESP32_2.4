/**
 * @file main_ui.cpp
 * Teensy 4.0 Cable Tester with LVGL UI and Screensaver
 * Display: ST7789VI 240x320 (8-bit parallel)
 * Touch: FT5x26 Capacitive
 * SD Card: Software SPI
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SdFat.h>
#include "display.h"
#include "ft6236.h"
#include "lvgl.h"
#include "lvgl_port.h"

// Cable configuration structure
typedef struct {
    uint8_t id;
    const char* name;
    uint16_t color;
} cable_config_t;

// Cable configurations
static const cable_config_t cable_configs[] = {
    {0x01, "HPT Standard", 0x4A9F},
    {0x02, "HPT Pro", 0xFD20},
    {0x03, "HPT Pro+", 0xF800},
    {0x04, "MPVI2", 0x05FF},
    {0x05, "MPVI2+", 0x07E0},
    {0x06, "MPVI3", 0xA81F},
    {0x07, "VCM Suite", 0xFD00},
    {0x08, "Custom Cable", 0xCE59},
};

#define NUM_CONFIGS (sizeof(cable_configs) / sizeof(cable_config_t))

// LVGL Objects
static lv_obj_t *main_screen;
static lv_obj_t *roller_cables;
static lv_obj_t *label_detected;
static lv_obj_t *panel;
static lv_obj_t *top_bar;
static lv_obj_t *label_selected;
static lv_style_t style_sel;

// Color profile cycling
typedef struct {
    uint32_t bg_color;
    uint32_t text_color;
    uint32_t accent_color;
} ui_color_profile_t;

static const ui_color_profile_t profiles[] = {
    {0x0A1428, 0xC0C0C0, 0x00A8FF},  // Dark blue, light gray, vibrant blue
    {0x1A0A1A, 0xD0D0D0, 0xFF00A8},  // Dark purple, light gray, magenta
    {0x0A1A0A, 0xC8C8C8, 0x00FF88},  // Dark green, light gray, cyan-green
    {0x1A1408, 0xC4C4C4, 0xFFB800},  // Dark orange, light gray, amber
    {0x141414, 0xB8B8B8, 0xFF6600},  // Dark gray, gray, orange
};
#define NUM_PROFILES (sizeof(profiles) / sizeof(ui_color_profile_t))

static int ui_color_profile = 0;
static uint32_t last_profile_change = 0;

// UI State
static uint8_t detected_cable_id = 0x00;

// Screensaver
// Screensaver timeout (disabled for testing - set to 300000 for 5 min)
#define SCREENSAVER_TIMEOUT_MS 300000
static uint32_t last_touch_time = 0;
static bool screensaver_active = false;

// Nyan cat animation
#define NYAN_WIDTH 240
#define NYAN_HEIGHT 320
#define NUM_FRAMES 12
#define FRAME_DELAY_MS 0
static uint16_t* frame_buffer = NULL;
static int current_frame = 0;
static uint32_t last_frame_time = 0;

// SD Card (Software SPI)
#define SD_CS   0
#define SD_MOSI 1
#define SD_MISO 2
#define SD_SCK  7
SdFat SD;
SdFile nyan_file;

// Touch controller pins (Software I2C)
#define TOUCH_SDA 17
#define TOUCH_SCL 16
#define TOUCH_INT 32
#define TOUCH_RST 8

// Display pins (8-bit parallel)
#define DISP_DC  9
#define DISP_CS  10
#define DISP_WR  13
#define DISP_RD  5
#define DISP_RST 8
#define DISP_BL  6
#define DISP_IM0 4
#define DISP_IM2 3

// Touch time update (called from LVGL port)
void update_touch_time(void) {
    last_touch_time = millis();
    
    if (screensaver_active) {
        screensaver_active = false;
        Serial.println("[MAIN] *** SCREENSAVER EXITED - Returning to UI ***");
        
        display_fill_screen(DISPLAY_BLACK);
        
        if (main_screen != NULL) {
            lv_scr_load(main_screen);
            lv_obj_invalidate(main_screen);
        }
    }
}

// Software I2C implementation
static void soft_i2c_init() {
    pinMode(TOUCH_SDA, INPUT_PULLUP);
    pinMode(TOUCH_SCL, INPUT_PULLUP);
}

static void soft_i2c_start() {
    pinMode(TOUCH_SDA, OUTPUT);
    digitalWrite(TOUCH_SDA, LOW);
    delayMicroseconds(2);
    digitalWrite(TOUCH_SCL, LOW);
    delayMicroseconds(2);
}

static void soft_i2c_stop() {
    pinMode(TOUCH_SDA, OUTPUT);
    digitalWrite(TOUCH_SDA, LOW);
    delayMicroseconds(2);
    pinMode(TOUCH_SCL, INPUT_PULLUP);
    delayMicroseconds(2);
    pinMode(TOUCH_SDA, INPUT_PULLUP);
    delayMicroseconds(2);
}

static bool soft_i2c_write_byte(uint8_t data) {
    for (int i = 7; i >= 0; i--) {
        digitalWrite(TOUCH_SDA, (data >> i) & 1);
        delayMicroseconds(2);
        pinMode(TOUCH_SCL, INPUT_PULLUP);
        delayMicroseconds(2);
        digitalWrite(TOUCH_SCL, LOW);
        delayMicroseconds(2);
    }
    
    pinMode(TOUCH_SDA, INPUT_PULLUP);
    delayMicroseconds(2);
    pinMode(TOUCH_SCL, INPUT_PULLUP);
    delayMicroseconds(2);
    bool ack = (digitalRead(TOUCH_SDA) == LOW);
    digitalWrite(TOUCH_SCL, LOW);
    delayMicroseconds(2);
    pinMode(TOUCH_SDA, OUTPUT);
    
    return ack;
}

static uint8_t soft_i2c_read_byte(bool ack) {
    uint8_t data = 0;
    pinMode(TOUCH_SDA, INPUT_PULLUP);
    
    for (int i = 7; i >= 0; i--) {
        delayMicroseconds(2);
        pinMode(TOUCH_SCL, INPUT_PULLUP);
        delayMicroseconds(2);
        if (digitalRead(TOUCH_SDA)) {
            data |= (1 << i);
        }
        digitalWrite(TOUCH_SCL, LOW);
        delayMicroseconds(2);
    }
    
    pinMode(TOUCH_SDA, OUTPUT);
    digitalWrite(TOUCH_SDA, ack ? LOW : HIGH);
    delayMicroseconds(2);
    pinMode(TOUCH_SCL, INPUT_PULLUP);
    delayMicroseconds(2);
    digitalWrite(TOUCH_SCL, LOW);
    delayMicroseconds(2);
    
    return data;
}

// Read cable ID (placeholder)
static uint8_t read_cable_id(void) {
    // TODO: Implement actual cable ID detection
    return 0x00;
}

// LVGL roller event handler
static void roller_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t selected = lv_roller_get_selected(obj);
        Serial.print("[MAIN] Selected cable: ");
        Serial.println(cable_configs[selected].name);

        lv_label_set_text(label_selected, cable_configs[selected].name);
        update_touch_time();  // Reset screensaver timer on interaction
    }
}

// Update UI color profile
static void update_color_profile(void)
{
    ui_color_profile = (ui_color_profile + 1) % NUM_PROFILES;
    
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(profiles[ui_color_profile].bg_color), 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(profiles[ui_color_profile].bg_color), 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(profiles[ui_color_profile].accent_color), 0);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(profiles[ui_color_profile].accent_color), 0);
    lv_obj_set_style_text_color(label_selected, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(roller_cables, lv_color_hex(profiles[ui_color_profile].bg_color), 0);
    lv_obj_set_style_text_color(roller_cables, lv_color_hex(profiles[ui_color_profile].text_color), 0);
    lv_obj_set_style_border_color(roller_cables, lv_color_hex(profiles[ui_color_profile].accent_color), 0);
    lv_style_set_bg_color(&style_sel, lv_color_hex(profiles[ui_color_profile].accent_color));
    lv_style_set_border_color(&style_sel, lv_color_hex(profiles[ui_color_profile].accent_color));
}

// Create LVGL UI
static void create_ui(void)
{
    main_screen = lv_screen_active();

    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x0A1428), 0);
    lv_obj_set_style_bg_opa(main_screen, LV_OPA_COVER, 0);

    // Top bar
    top_bar = lv_obj_create(main_screen);
    lv_obj_set_size(top_bar, 240, 40);
    lv_obj_align(top_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x00A8FF), 0);
    lv_obj_set_style_bg_opa(top_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);
    lv_obj_set_style_radius(top_bar, 0, 0);
    lv_obj_set_style_pad_all(top_bar, 0, 0);

    // Selected cable label
    label_selected = lv_label_create(top_bar);
    lv_label_set_text(label_selected, cable_configs[0].name);
    lv_obj_set_style_text_font(label_selected, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label_selected, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(label_selected, LV_OPA_TRANSP, 0);
    lv_obj_center(label_selected);

    // Panel
    panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, 220, 260);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x0A1428), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_80, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x00A8FF), 0);
    lv_obj_set_style_border_width(panel, 2, 0);
    lv_obj_set_style_radius(panel, 10, 0);

    // Detected cable label
    label_detected = lv_label_create(panel);
    lv_label_set_text(label_detected, "No cable detected");
    lv_obj_set_style_text_font(label_detected, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_detected, lv_color_hex(0xFF4444), 0);
    lv_obj_set_style_bg_opa(label_detected, LV_OPA_TRANSP, 0);
    lv_obj_align(label_detected, LV_ALIGN_TOP_LEFT, 10, 10);

    // Build roller options
    static char roller_opts[512];
    roller_opts[0] = '\0';
    for (int i = 0; i < NUM_CONFIGS; i++) {
        if (i > 0) strcat(roller_opts, "\n");
        strcat(roller_opts, cable_configs[i].name);
    }

    // Roller widget
    roller_cables = lv_roller_create(panel);
    lv_roller_set_options(roller_cables, roller_opts, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_cables, 5);
    lv_obj_set_width(roller_cables, 200);
    lv_obj_align(roller_cables, LV_ALIGN_CENTER, 0, 5);
    lv_obj_set_style_anim_duration(roller_cables, 0, 0);
    lv_obj_set_style_bg_color(roller_cables, lv_color_hex(0x0A1428), 0);
    lv_obj_set_style_text_color(roller_cables, lv_color_hex(0xC0C0C0), 0);
    lv_obj_set_style_border_color(roller_cables, lv_color_hex(0x00A8FF), 0);
    lv_obj_set_style_border_width(roller_cables, 1, 0);

    // Selected item style
    lv_style_init(&style_sel);
    lv_style_set_text_font(&style_sel, &lv_font_montserrat_20);
    lv_style_set_bg_color(&style_sel, lv_color_hex(0x00A8FF));
    lv_style_set_bg_opa(&style_sel, LV_OPA_50);
    lv_style_set_text_color(&style_sel, lv_color_hex(0xFFFFFF));
    lv_style_set_border_width(&style_sel, 2);
    lv_style_set_border_color(&style_sel, lv_color_hex(0x00D4FF));
    lv_obj_add_style(roller_cables, &style_sel, LV_PART_SELECTED);

    lv_obj_add_event_cb(roller_cables, roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    Serial.println("[MAIN] LVGL UI created");
}

// Update detected cable label
static void update_detected_cable(uint8_t cable_id)
{
    if (cable_id == 0x00) {
        lv_label_set_text(label_detected, "No cable detected");
        lv_obj_set_style_text_color(label_detected, lv_color_hex(0xFF4444), 0);
    } else {
        char text[64];
        snprintf(text, sizeof(text), "Detected: ID 0x%02X", cable_id);
        lv_label_set_text(label_detected, text);
        lv_obj_set_style_text_color(label_detected, lv_color_hex(0x00FF88), 0);
    }
}

// Draw Nyan screensaver
static void draw_nyan_screensaver(void) {
    uint32_t now = millis();
    
    if (now - last_frame_time < FRAME_DELAY_MS) {
        return;
    }
    
    last_frame_time = now;
    
    // Load frame from SD card
    char filename[32];
    snprintf(filename, sizeof(filename), "nyan_%d.raw", current_frame);
    
    if (nyan_file.open(filename, O_RDONLY)) {
        if (frame_buffer == NULL) {
            frame_buffer = (uint16_t*)malloc(NYAN_WIDTH * NYAN_HEIGHT * 2);
        }
        
        if (frame_buffer != NULL) {
            size_t bytes_read = nyan_file.read(frame_buffer, NYAN_WIDTH * NYAN_HEIGHT * 2);
            
            if (bytes_read == NYAN_WIDTH * NYAN_HEIGHT * 2) {
                display_set_addr_window(0, 0, NYAN_WIDTH - 1, NYAN_HEIGHT - 1);
                display_write_pixels(frame_buffer, NYAN_WIDTH * NYAN_HEIGHT);
            }
        }
        
        nyan_file.close();
    }
    
    current_frame = (current_frame + 1) % NUM_FRAMES;
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    
    Serial.println("\n========================================");
    Serial.println("Teensy 4.0 Cable Tester with LVGL UI");
    Serial.println("========================================\n");

    // Initialize display (16-bit parallel mode - same config that worked in test)
    display_config_t display_config = {
        .pin_data = {19, 18, 14, 15, 28, 29, 30, 31, 22, 23, 20, 21, 24, 25, 26, 27},  // DB0-DB15 for 16-bit mode
        .pin_dc = DISP_DC,
        .pin_cs = DISP_CS,
        .pin_wr = DISP_WR,
        .pin_rd = DISP_RD,
        .pin_rst = DISP_RST,
        .pin_bl = DISP_BL,
        .pin_im0 = DISP_IM0,
        .pin_im2 = DISP_IM2,
        .use_16bit = true
    };
    
    if (!display_init(&display_config)) {
        Serial.println("[MAIN] ERROR: Display initialization failed!");
        while(1) delay(1000);
    }
    
    display_fill_screen(DISPLAY_BLACK);
    display_on();
    Serial.println("[MAIN] Display initialized");
    
    // Test color bars before starting UI
    Serial.println("[MAIN] Drawing color bar test...");
    const uint16_t colors[] = {
        DISPLAY_RED,     // 0xF800
        DISPLAY_GREEN,   // 0x07E0
        DISPLAY_BLUE,    // 0x001F
        DISPLAY_YELLOW,  // 0xFFE0
        DISPLAY_CYAN,    // 0x07FF
        DISPLAY_MAGENTA, // 0xF81F
        DISPLAY_WHITE,   // 0xFFFF
        DISPLAY_BLACK    // 0x0000
    };
    for (int i = 0; i < 8; i++) {
        display_fill_rect(0, i * 40, 240, 40, colors[i]);
        Serial.print("[MAIN] Bar ");
        Serial.print(i);
        Serial.print(" color: 0x");
        Serial.println(colors[i], HEX);
    }
    delay(3000);  // Show color bars for 3 seconds
    display_fill_screen(DISPLAY_BLACK);
    Serial.println("[MAIN] Color bar test complete");

    // Initialize touch
    soft_i2c_init();
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_RST, HIGH);
    delay(100);
    
    ft5x26_config_t touch_config = {
        .pin_sda = TOUCH_SDA,
        .pin_scl = TOUCH_SCL,
        .pin_int = TOUCH_INT,
        .pin_rst = TOUCH_RST
    };
    
    if (!ft5x26_init(&touch_config)) {
        Serial.println("[MAIN] WARNING: Touch controller initialization failed");
    } else {
        Serial.println("[MAIN] Touch controller initialized");
    }

    // Initialize SD card
    SPI.setMOSI(SD_MOSI);
    SPI.setMISO(SD_MISO);
    SPI.setSCK(SD_SCK);
    
    if (!SD.begin(SD_CS, SD_SCK_MHZ(4))) {
        Serial.println("[MAIN] WARNING: SD card initialization failed");
    } else {
        Serial.println("[MAIN] SD card initialized");
    }

    // Initialize LVGL
    Serial.println("[MAIN] Initializing LVGL...");
    if (!lvgl_port_init()) {
        Serial.println("[MAIN] ERROR: LVGL port initialization failed!");
        while(1) delay(1000);
    }
    Serial.println("[MAIN] LVGL initialized");

    // Create UI
    Serial.println("[MAIN] Creating UI...");
    last_touch_time = millis();
    last_profile_change = last_touch_time;
    create_ui();
    Serial.println("[MAIN] UI created");
    
    // Force LVGL to render immediately
    Serial.println("[MAIN] Forcing initial LVGL render...");
    for (int i = 0; i < 10; i++) {
        lv_tick_inc(5);
        lv_timer_handler();
        delay(5);
    }
    Serial.println("[MAIN] Initial render complete");

    detected_cable_id = read_cable_id();
    update_detected_cable(detected_cable_id);

    Serial.println("[MAIN] System ready!");
    Serial.print("[MAIN] Screensaver activates after ");
    Serial.print(SCREENSAVER_TIMEOUT_MS);
    Serial.println(" ms of inactivity");
}

void loop() {
    static uint32_t last_id_check = 0;
    static uint32_t last_debug_time = 0;
    uint32_t now = millis();

    // Color profile cycling disabled - keeping static color scheme
    // if (!screensaver_active) {
    //     if (now - last_profile_change > 5000) {
    //         update_color_profile();
    //         last_profile_change = now;
    //     }
    // }

    // Check for cable ID changes
    if (now - last_id_check > 1000) {
        uint8_t new_id = read_cable_id();
        if (new_id != detected_cable_id) {
            detected_cable_id = new_id;
            update_detected_cable(detected_cable_id);
            Serial.print("[MAIN] Cable ID changed: 0x");
            Serial.println(detected_cable_id, HEX);
        }
        last_id_check = now;
    }

    // Check for screensaver timeout
    uint32_t idle_time = now - last_touch_time;
    
    if (now - last_debug_time > 5000) {
        Serial.print("[MAIN] Idle: ");
        Serial.print(idle_time);
        Serial.print(" ms, Active: ");
        Serial.print(screensaver_active);
        Serial.print(", Flush count: ");
        Serial.println(lvgl_flush_count);
        last_debug_time = now;
    }

    if (!screensaver_active && idle_time > SCREENSAVER_TIMEOUT_MS) {
        screensaver_active = true;
        Serial.print("[MAIN] *** SCREENSAVER ACTIVATED after ");
        Serial.print(idle_time);
        Serial.println(" ms idle ***");
        display_fill_screen(DISPLAY_BLACK);
        current_frame = 0;
    }

    // Draw screensaver or run LVGL
    if (screensaver_active) {
        // Check for touch to exit
        ft5x26_touch_t touch_data;
        if (ft5x26_read_touch(&touch_data) && touch_data.touch_count > 0) {
            update_touch_time();
            if (!screensaver_active) {
                lvgl_port_task_handler();
            }
        } else {
            draw_nyan_screensaver();
        }
    } else {
        lvgl_port_task_handler();
        delay(10);
    }
}
