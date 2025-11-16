#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "ili9341.h"
#include "ft6236.h"
#include "sd_spi.h"
#include "lvgl.h"
#include "lvgl_port.h"

static const char *TAG = "CABLE_CONFIG";

// Cable configuration structure
typedef struct {
    uint8_t id;
    const char* name;
    uint16_t color;
} cable_config_t;

// Cable configurations (placeholder data)
static const cable_config_t cable_configs[] = {
    {0x01, "USB-C to USB-A", 0x4A9F},      // Teal
    {0x02, "USB-C to Lightning", 0xFD20},  // Orange
    {0x03, "HDMI Standard", 0xF800},       // Red
    {0x04, "DisplayPort", 0x05FF},         // Cyan
    {0x05, "Ethernet RJ45", 0x07E0},       // Green
    {0x06, "USB-A to Micro", 0xA81F},      // Purple
    {0x07, "Audio 3.5mm", 0xFD00},         // Yellow-Orange
    {0x08, "Power Barrel", 0xCE59},        // Light Gray
};

#define NUM_CONFIGS (sizeof(cable_configs) / sizeof(cable_config_t))

// LVGL Objects
static lv_obj_t *main_screen;
static lv_obj_t *roller_cables;
static lv_obj_t *label_detected;
static lv_obj_t *label_status;
static lv_obj_t *panel;
static lv_obj_t *label_title;
static lv_style_t style_sel;

// Color profile cycling
static int ui_color_profile = 0;
static int64_t last_profile_change = 0;

// UI State
static uint8_t detected_cable_id = 0x00;  // Will be read from IC

// Screensaver
#define SCREENSAVER_TIMEOUT_MS 10000
static int64_t last_touch_time = 0;
static bool screensaver_active = false;

// Nyan cat animation - Full screen 320x240
#define NYAN_WIDTH 320
#define NYAN_HEIGHT 240
#define CHUNK_LINES 40  // Load 40 lines at a time (25,600 bytes per chunk)
static uint16_t* chunk_buffer = NULL;  // Small buffer for streaming
static int current_frame = 0;

// Update touch time (called from LVGL port layer)
void update_touch_time(void) {
    last_touch_time = esp_timer_get_time() / 1000;
    
    // Exit screensaver on touch
    if (screensaver_active) {
        screensaver_active = false;
        ESP_LOGI(TAG, "*** SCREENSAVER EXITED - Returning to Rolodex ***");
        
        // Clear screen and restore LVGL UI
        ili9341_fill_screen(ILI9341_BLACK);
        
        // Force LVGL to fully redraw the main screen
        if (main_screen != NULL) {
            lv_scr_load(main_screen);  // Reload main screen
            lv_obj_invalidate(main_screen);  // Trigger full redraw
        }
    }
}

// Pin configuration - ER-TFTM024-3 to ESP32-S3 (4-wire SPI + I2C Touch + SD Card)
// Ordered by ESP32-S3 DevKit pin layout for easy wiring verification

// TFT LCD (ILI9341) - 4-Wire SPI
#define TFT_BL      4   // GPIO 4  → Pin 29 BL_ON/OFF (Backlight)
#define TFT_SCLK    6   // GPIO 6  → Pin 24 D/C(SCL) - SPI Clock in 4-wire mode
#define TFT_MOSI    11  // GPIO 11 → Pin 27 LCD_SDI (SPI MOSI)
#define TFT_DC      12  // GPIO 12 → Pin 25 /WR(D/C) - Data/Command in 4-wire SPI mode
#define TFT_MISO    13  // GPIO 13 → Pin 28 LCD_SDO (SPI MISO, optional)
#define TFT_CS      15  // GPIO 15 → Pin 23 LCD_/CS (Chip Select)
#define TFT_RST     -1  // Not connected (Pin 21 has onboard RC reset)

// CTP Touch (FT6236) - I2C
#define TOUCH_SDA   8   // GPIO 8  → Pin 31 CTP_SDA (I2C Touch Data)
#define TOUCH_SCL   9   // GPIO 9  → Pin 30 CTP_SCL (I2C Touch Clock)
#define TOUCH_INT   3   // GPIO 3  → Pin 39 CTP_INT (Touch Interrupt, optional)

// SD Card - Separate SPI3 Bus
#define SD_CS       16  // GPIO 16 - SD Card Chip Select
#define SD_MOSI     35  // GPIO 35 - SD Card MOSI
#define SD_MISO     36  // GPIO 36 - SD Card MISO
#define SD_SCK      18  // GPIO 18 - SD Card Clock

// RGB LED - WS2812 on GPIO 48
#define RGB_LED_PIN 48  // GPIO 48 - WS2812 RGB LED

static void draw_nyan_screensaver(void) {
    static bool bg_drawn = false;
    
    // Draw dark background once
    if (!bg_drawn) {
        ili9341_fill_screen(0x0000);  // Black background
        bg_drawn = true;
    }
    
    // Allocate small chunk buffer once (40 lines = 25,600 bytes)
    if (chunk_buffer == NULL) {
        chunk_buffer = (uint16_t*)malloc(NYAN_WIDTH * CHUNK_LINES * 2);
        if (chunk_buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate chunk buffer (%d bytes)", NYAN_WIDTH * CHUNK_LINES * 2);
            return;
        }
        ESP_LOGI(TAG, "Allocated chunk buffer: %d bytes for %d lines", NYAN_WIDTH * CHUNK_LINES * 2, CHUNK_LINES);
    }
    
    // Stream current frame from SD card in chunks
    if (sd_mount()) {
        char filename[32];
        snprintf(filename, sizeof(filename), "nyan_%d.raw", current_frame);
        
        extern void ili9341_write_pixels(const uint16_t* pixels, uint32_t length);
        
        // Stream image in 40-line chunks (6 chunks for 240 lines)
        for (int y = 0; y < NYAN_HEIGHT; y += CHUNK_LINES) {
            uint32_t offset = y * NYAN_WIDTH * 2;  // Byte offset in file
            uint32_t chunk_size = NYAN_WIDTH * CHUNK_LINES * 2;  // Bytes to read
            
            // Read chunk from SD card
            if (sd_read_chunk(filename, offset, (uint8_t*)chunk_buffer, chunk_size)) {
                // Apply Swap+Invert color transformation
                for (int i = 0; i < NYAN_WIDTH * CHUNK_LINES; i++) {
                    uint16_t pixel = chunk_buffer[i];
                    pixel = (pixel >> 8) | (pixel << 8);  // Byte swap
                    chunk_buffer[i] = ~pixel;              // Invert
                }
                
                // Set window for this chunk
                ili9341_set_addr_window(0, y, NYAN_WIDTH - 1, y + CHUNK_LINES - 1);
                // Write chunk to display
                ili9341_write_pixels(chunk_buffer, NYAN_WIDTH * CHUNK_LINES);
            } else {
                ESP_LOGE(TAG, "Failed to read chunk from %s at offset %d", filename, offset);
                break;
            }
        }
    } else {
        ESP_LOGE(TAG, "SD card not mounted!");
    }
    
    // Check for touch to exit screensaver
    ft6236_touch_t touch_data;
    if (ft6236_read_touch(&touch_data)) {
        ESP_LOGI(TAG, "Touch detected during screensaver, exiting");
        screensaver_active = false;
        bg_drawn = false;  // Reset for next time
        
        // Free buffer when exiting screensaver to save RAM
        if (chunk_buffer != NULL) {
            free(chunk_buffer);
            chunk_buffer = NULL;
        }
        
        lv_obj_invalidate(main_screen);  // Redraw UI
        update_touch_time();  // Reset timer
    }
    
    // Advance to next frame every draw (maximum speed)
    current_frame = (current_frame + 1) % 12;  // 12 frames total
}

// Read cable ID from IC (placeholder - implement based on your IC interface)
static uint8_t read_cable_id(void) {
    // TODO: Implement actual IC communication
    // For now, return a test value
    static uint8_t test_id = 0x00;
    
    // Simulate detection every few seconds
    static uint32_t last_read = 0;
    uint32_t now = xTaskGetTickCount() / portTICK_PERIOD_MS;
    
    if (now - last_read > 5000) {  // Check every 5 seconds
        test_id = (test_id == 0x00) ? 0x42 : 0x00;
        last_read = now;
    }
    
    return test_id;
}

// LVGL Event handler for roller selection
static void roller_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t selected = lv_roller_get_selected(obj);
        ESP_LOGI(TAG, "Selected cable: %s", cable_configs[selected].name);
        
        // Update status label
        char status_text[64];
        snprintf(status_text, sizeof(status_text), "Selected: %s", cable_configs[selected].name);
        lv_label_set_text(label_status, status_text);
    }
}

// Apply color profile to UI
static void apply_color_profile(int profile) {
    // Define 5 color profiles
    const struct {
        const char* name;
        uint32_t bg_color;
        uint32_t accent_color;
        uint32_t text_color;
        uint32_t error_color;
    } profiles[] = {
        {"Dark Blue",   0x0A1428, 0x00A8FF, 0xC0C0C0, 0xFF4444},  // Original
        {"Purple Dark", 0x1A0A28, 0xA855F7, 0xE0D0FF, 0xFF6B6B},  // Purple theme
        {"Green Dark",  0x0A1F14, 0x10B981, 0xD1FAE5, 0xF87171},  // Green theme
        {"Orange Dark", 0x1F1408, 0xF59E0B, 0xFED7AA, 0xEF4444},  // Orange theme
        {"Cyan Dark",   0x08191F, 0x06B6D4, 0xCFFAFE, 0xF87171},  // Cyan theme
    };
    
    ESP_LOGI(TAG, ">>> UI COLOR PROFILE: %s <<<", profiles[profile].name);
    
    // Update screen background
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(profiles[profile].bg_color), 0);
    
    // Update panel colors
    lv_obj_set_style_bg_color(panel, lv_color_hex(profiles[profile].bg_color), 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(profiles[profile].accent_color), 0);
    
    // Update title color
    lv_obj_set_style_text_color(label_title, lv_color_hex(profiles[profile].accent_color), 0);
    
    // Update status color
    lv_obj_set_style_text_color(label_status, lv_color_hex(profiles[profile].accent_color), 0);
    
    // Update roller colors
    lv_obj_set_style_bg_color(roller_cables, lv_color_hex(profiles[profile].bg_color), 0);
    lv_obj_set_style_text_color(roller_cables, lv_color_hex(profiles[profile].text_color), 0);
    lv_obj_set_style_border_color(roller_cables, lv_color_hex(profiles[profile].accent_color), 0);
    
    // Update selected item style
    lv_style_set_bg_color(&style_sel, lv_color_hex(profiles[profile].accent_color));
    lv_style_set_border_color(&style_sel, lv_color_hex(profiles[profile].accent_color));
}

// Create LVGL UI
static void create_ui(void)
{
    // Save reference to main screen
    main_screen = lv_screen_active();
    
    // Set screen to dark blue background
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x0A1428), 0);
    lv_obj_set_style_bg_opa(main_screen, LV_OPA_COVER, 0);
    
    // Create semi-transparent dark panel for UI
    panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, 300, 220);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x0A1428), 0);  // Dark blue
    lv_obj_set_style_bg_opa(panel, LV_OPA_80, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x00A8FF), 0);  // Vibrant blue
    lv_obj_set_style_border_width(panel, 2, 0);
    lv_obj_set_style_radius(panel, 10, 0);
    
    // Create title label
    label_title = lv_label_create(panel);
    lv_label_set_text(label_title, "Cable Configuration");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_title, lv_color_hex(0x00A8FF), 0);  // Vibrant blue
    lv_obj_set_style_bg_opa(label_title, LV_OPA_TRANSP, 0);  // Transparent background
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Create detected cable status label
    label_detected = lv_label_create(panel);
    lv_label_set_text(label_detected, "No cable detected");
    lv_obj_set_style_text_font(label_detected, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_detected, lv_color_hex(0xFF4444), 0);  // Red
    lv_obj_set_style_bg_opa(label_detected, LV_OPA_TRANSP, 0);  // Transparent background
    lv_obj_align(label_detected, LV_ALIGN_TOP_LEFT, 10, 45);
    
    // Build options string for roller
    static char roller_opts[512];
    roller_opts[0] = '\0';
    for (int i = 0; i < NUM_CONFIGS; i++) {
        if (i > 0) strcat(roller_opts, "\n");
        strcat(roller_opts, cable_configs[i].name);
    }
    
    // Create roller widget
    roller_cables = lv_roller_create(panel);
    lv_roller_set_options(roller_cables, roller_opts, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_cables, 4);
    lv_obj_set_width(roller_cables, 260);
    lv_obj_align(roller_cables, LV_ALIGN_CENTER, 0, 20);
    
    // Set roller to instant scrolling with no animations
    lv_obj_set_style_anim_duration(roller_cables, 0, 0);  // No animation delay
    
    // Dark theme style for roller
    lv_obj_set_style_bg_color(roller_cables, lv_color_hex(0x0A1428), 0);
    lv_obj_set_style_text_color(roller_cables, lv_color_hex(0xC0C0C0), 0);  // Light gray text
    lv_obj_set_style_border_color(roller_cables, lv_color_hex(0x00A8FF), 0);
    lv_obj_set_style_border_width(roller_cables, 1, 0);
    
    // Style the selected item
    lv_style_init(&style_sel);
    lv_style_set_text_font(&style_sel, &lv_font_montserrat_22);
    lv_style_set_bg_color(&style_sel, lv_color_hex(0x00A8FF));  // Vibrant blue
    lv_style_set_bg_opa(&style_sel, LV_OPA_50);
    lv_style_set_text_color(&style_sel, lv_color_hex(0xFFFFFF));  // White text
    lv_style_set_border_width(&style_sel, 2);
    lv_style_set_border_color(&style_sel, lv_color_hex(0x00D4FF));  // Lighter blue
    lv_obj_add_style(roller_cables, &style_sel, LV_PART_SELECTED);
    
    // Add event handler
    lv_obj_add_event_cb(roller_cables, roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Create status label at bottom
    label_status = lv_label_create(panel);
    lv_label_set_text(label_status, "Swipe to select cable type");
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_status, lv_color_hex(0x00A8FF), 0);  // Vibrant blue
    lv_obj_set_style_bg_opa(label_status, LV_OPA_TRANSP, 0);  // Transparent background
    lv_obj_align(label_status, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    ESP_LOGI(TAG, "LVGL UI created");
}

// Update UI with detected cable
static void update_detected_cable(uint8_t cable_id)
{
    if (cable_id == 0x00) {
        lv_label_set_text(label_detected, "No cable detected");
        lv_obj_set_style_text_color(label_detected, lv_color_hex(0xFF4444), 0);  // Red
    } else {
        char text[64];
        snprintf(text, sizeof(text), "Detected: ID 0x%02X", cable_id);
        lv_label_set_text(label_detected, text);
        lv_obj_set_style_text_color(label_detected, lv_color_hex(0x00FF88), 0);  // Green
    }
}

// Display boot screen with HPTuners logo
static void show_boot_screen(void) {
    ESP_LOGI(TAG, "=== BOOT SCREEN START ===");
    
    // Turn off backlight while loading to avoid flash
    extern void ili9341_set_backlight(uint8_t brightness);
    ili9341_set_backlight(0);
    
    // Load and display full boot splash screen using chunk streaming (like screensaver)
    if (sd_mount()) {
        #define SPLASH_WIDTH 320
        #define SPLASH_HEIGHT 240
        #define SPLASH_CHUNK_LINES 40  // Load 40 lines at a time (25,600 bytes)
        
        uint16_t* splash_chunk = (uint16_t*)malloc(SPLASH_WIDTH * SPLASH_CHUNK_LINES * 2);
        if (splash_chunk != NULL) {
            ESP_LOGI(TAG, "Loading boot splash in %d-line chunks...", SPLASH_CHUNK_LINES);
            
            bool success = true;
            
            // Stream the image in chunks (same as screensaver)
            for (int y = 0; y < SPLASH_HEIGHT; y += SPLASH_CHUNK_LINES) {
                uint32_t offset = y * SPLASH_WIDTH * 2;  // Byte offset in file
                uint32_t chunk_size = SPLASH_WIDTH * SPLASH_CHUNK_LINES * 2;  // Bytes to read
                
                if (!sd_read_chunk("hpt_logo.raw", offset, (uint8_t*)splash_chunk, chunk_size)) {
                    ESP_LOGE(TAG, "Failed to read chunk at offset %d", offset);
                    success = false;
                    break;
                }
                
                // Display this chunk (no color transform - already baked in)
                ili9341_set_addr_window(0, y, SPLASH_WIDTH - 1, y + SPLASH_CHUNK_LINES - 1);
                extern void ili9341_write_pixels(const uint16_t* pixels, uint32_t length);
                ili9341_write_pixels(splash_chunk, SPLASH_WIDTH * SPLASH_CHUNK_LINES);
            }
            
            if (success) {
                ESP_LOGI(TAG, "Boot splash displayed successfully");
            } else {
                ESP_LOGE(TAG, "Boot splash incomplete - showing red screen");
                ili9341_fill_screen(0xF800);
            }
            
            free(splash_chunk);
        } else {
            ESP_LOGE(TAG, "Failed to allocate %d byte chunk buffer - showing blue screen", SPLASH_WIDTH * SPLASH_CHUNK_LINES * 2);
            ili9341_fill_screen(0x001F);
        }
    } else {
        ESP_LOGW(TAG, "SD card not mounted - showing green screen");
        ili9341_fill_screen(0x07E0);
    }
    
    // Turn on backlight now that image is displayed
    ili9341_set_backlight(255);
    
    ESP_LOGI(TAG, "Waiting for touch to start...");
    
    // Clear any pending touches
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Wait for touch before continuing
    bool touched = false;
    int check_count = 0;
    while (!touched) {
        if (ft6236_is_touched()) {
            touched = true;
            ESP_LOGI(TAG, "Touch detected after %d checks - starting application", check_count);
        }
        check_count++;
        if (check_count % 20 == 0) {
            ESP_LOGI(TAG, "Still waiting for touch... (%d checks)", check_count);
        }
        vTaskDelay(pdMS_TO_TICKS(50));  // Poll every 50ms
    }
    
    // Brief delay to debounce
    vTaskDelay(pdMS_TO_TICKS(200));
    ESP_LOGI(TAG, "=== BOOT SCREEN END ===");
}

// Simple gradient background demo
void app_main(void) {
    // Wait 3 seconds for serial monitor to connect
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32-S3 ILI9341 + FT6236 Touch Demo");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    
    // Initialize display FIRST (this initializes SPI2_HOST bus)
    ili9341_config_t display_config = {
        .pin_mosi = TFT_MOSI,
        .pin_miso = TFT_MISO,
        .pin_sclk = TFT_SCLK,
        .pin_cs = TFT_CS,
        .pin_dc = TFT_DC,
        .pin_rst = TFT_RST,
        .pin_bl = TFT_BL,
        .spi_host = SPI2_HOST,
        .spi_clock_mhz = 80  // 80MHz SPI clock - maximum speed
    };
    
    if (!ili9341_init(&display_config)) {
        ESP_LOGE(TAG, "Display initialization failed!");
        return;
    }
    
    ESP_LOGI(TAG, "SD card initialized successfully");
    
    // Initialize SD card on SPI3
    ESP_LOGI(TAG, "Initializing SD card on SPI3 (CS=%d, MOSI=%d, MISO=%d, CLK=%d)", 
             SD_CS, SD_MOSI, SD_MISO, SD_SCK);
    
    // Try to initialize SD card, but don't block if it fails
    if (!sd_init(SD_CS, SD_MOSI, SD_MISO, SD_SCK)) {
        ESP_LOGW(TAG, "SD card initialization failed - will retry later");
        ESP_LOGW(TAG, "Boot screen and screensaver may not work until SD card is ready");
    } else {
        ESP_LOGI(TAG, "SD card initialized successfully");
    }
    
    // Initialize touch controller BEFORE boot screen
    ft6236_config_t touch_config = {
        .i2c_port = I2C_NUM_0,
        .pin_sda = TOUCH_SDA,
        .pin_scl = TOUCH_SCL,
        .pin_int = TOUCH_INT,
        .i2c_freq = 400000  // 400kHz I2C
    };
    
    if (!ft6236_init(&touch_config)) {
        ESP_LOGE(TAG, "Touch controller initialization failed!");
    } else {
        ESP_LOGI(TAG, "Touch controller initialized successfully");
    }
    
    // Show boot screen with HPTuners logo
    ESP_LOGI(TAG, "*** ABOUT TO CALL show_boot_screen() ***");
    show_boot_screen();
    ESP_LOGI(TAG, "*** show_boot_screen() RETURNED ***");
    
    // Initialize touch time BEFORE LVGL to prevent screensaver triggering immediately
    last_touch_time = esp_timer_get_time() / 1000;
    last_profile_change = last_touch_time;  // Initialize profile timer
    
    // Initialize LVGL
    ESP_LOGI(TAG, "Initializing LVGL...");
    if (!lvgl_port_init()) {
        ESP_LOGE(TAG, "LVGL initialization failed!");
        return;
    }
    
    // Create UI
    ESP_LOGI(TAG, "Creating UI...");
    create_ui();
    
    // Read initial cable ID
    detected_cable_id = read_cable_id();
    update_detected_cable(detected_cable_id);
    
    ESP_LOGI(TAG, "System ready! Use roller to select cable type.");
    ESP_LOGI(TAG, "Screensaver will activate after %d ms of inactivity", SCREENSAVER_TIMEOUT_MS);
    
    // Main loop with LVGL task handler
    uint32_t last_id_check = 0;
    
    while (1) {
        // Change UI color profile every 5 seconds (when not in screensaver)
        if (!screensaver_active) {
            int64_t current_time = esp_timer_get_time() / 1000;
            if (current_time - last_profile_change > 5000) {
                ui_color_profile = (ui_color_profile + 1) % 5;
                last_profile_change = current_time;
                apply_color_profile(ui_color_profile);
            }
        }
        
        // Periodically check for cable ID changes
        uint32_t now = xTaskGetTickCount() / portTICK_PERIOD_MS;
        if (now - last_id_check > 1000) {  // Check every second
            uint8_t new_id = read_cable_id();
            if (new_id != detected_cable_id) {
                detected_cable_id = new_id;
                update_detected_cable(detected_cable_id);
                ESP_LOGI(TAG, "Cable ID changed: 0x%02X", detected_cable_id);
            }
            last_id_check = now;
        }
        
        // Check for screensaver timeout
        int64_t current_time = esp_timer_get_time() / 1000;
        int64_t idle_time = current_time - last_touch_time;
        
        // Debug: Log idle time every 5 seconds
        static int64_t last_debug_time = 0;
        if (current_time - last_debug_time > 5000) {
            ESP_LOGI(TAG, "Idle: %lld ms, Active: %d, Timeout: %d ms", 
                     idle_time, screensaver_active, SCREENSAVER_TIMEOUT_MS);
            last_debug_time = current_time;
        }
        
        if (!screensaver_active && idle_time > SCREENSAVER_TIMEOUT_MS) {
            screensaver_active = true;
            ESP_LOGI(TAG, "*** SCREENSAVER ACTIVATED after %lld ms idle ***", idle_time);
            // Black out screen for screensaver
            ili9341_fill_screen(ILI9341_BLACK);
        }
        
        // Draw screensaver if active
        if (screensaver_active) {
            // Check for touch to exit screensaver
            ft6236_touch_t touch_data;
            if (ft6236_read_touch(&touch_data) && touch_data.touch_count > 0) {
                update_touch_time();  // This will exit screensaver
                // Give LVGL one cycle to redraw the UI
                if (!screensaver_active) {
                    lvgl_port_task_handler();
                }
            } else {
                draw_nyan_screensaver();
            }
            // No delay - maximum frame rate for screensaver
        } else {
            // Handle LVGL tasks (touch input, rendering, etc.)
            lvgl_port_task_handler();
            // No delay - maximum responsiveness
        }
    }
}
