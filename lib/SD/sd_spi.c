#include "sd_spi.h"
#include <string.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

static const char *TAG = "SD_SPI";
static sdmmc_card_t *card = NULL;
static bool sd_mounted = false;

#define MOUNT_POINT "/sdcard"

bool sd_init(int cs_pin, int mosi_pin, int miso_pin, int clk_pin) {
    ESP_LOGI(TAG, "Initializing SD card on SPI3");
    
    // Clean up any previous attempt
    if (sd_mounted) {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
        sd_mounted = false;
        card = NULL;
    }
    
    // Small delay to let card settle after power-on
    vTaskDelay(pdMS_TO_TICKS(100));

    // Initialize SPI3 bus for SD card (separate from display SPI2)
    ESP_LOGI(TAG, "SPI3 Pins: MOSI=%d, MISO=%d, CLK=%d, CS=%d", mosi_pin, miso_pin, clk_pin, cs_pin);
    
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = mosi_pin,
        .miso_io_num = miso_pin,
        .sclk_io_num = clk_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };
    
    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to initialize SPI3 bus: %s", esp_err_to_name(ret));
        return false;
    }
    ESP_LOGI(TAG, "SPI3 bus initialized successfully");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST;  // Use SPI3 instead of default SPI2
    host.max_freq_khz = 20000;  // 20MHz for fast reading
    
    ESP_LOGI(TAG, "Host config: slot=%d, max_freq=%d kHz", host.slot, host.max_freq_khz);

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = cs_pin;
    slot_config.host_id = SPI3_HOST;
    
    ESP_LOGI(TAG, "Slot config: CS=%d, host_id=%d", slot_config.gpio_cs, slot_config.host_id);

    ESP_LOGI(TAG, "Attempting to mount SD card on SPI3...");
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    
    ESP_LOGI(TAG, "Mount attempt result: %s (0x%x)", esp_err_to_name(ret), ret);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. Check SD card.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SD card: %s", esp_err_to_name(ret));
        }
        return false;
    }

    sd_mounted = true;

    // Card info
    ESP_LOGI(TAG, "SD card mounted successfully");
    sdmmc_card_print_info(stdout, card);

    return true;
}

bool sd_mount(void) {
    return sd_mounted;
}

void sd_unmount(void) {
    if (sd_mounted) {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
        spi_bus_free(SPI3_HOST);
        sd_mounted = false;
        ESP_LOGI(TAG, "SD card unmounted");
    }
}

// Swap R and B channels in RGB565 format
static inline uint16_t swap_rb(uint16_t color) {
    return ((color & 0x001F) << 11) | (color & 0x07E0) | ((color & 0xF800) >> 11);
}

// Invert color (for negative images)
static inline uint16_t invert_color(uint16_t color) {
    return ~color;
}

bool sd_load_image(const char* filename, uint16_t* buffer, uint32_t max_size) {
    if (!sd_mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return false;
    }

    char filepath[64];
    snprintf(filepath, sizeof(filepath), "%s/%s", MOUNT_POINT, filename);

    FILE* f = fopen(filepath, "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        return false;
    }

    // Read file
    size_t bytes_read = fread(buffer, 1, max_size, f);
    fclose(f);

    if (bytes_read == 0) {
        ESP_LOGE(TAG, "Failed to read file: %s", filepath);
        return false;
    }

    ESP_LOGI(TAG, "Loaded %d bytes from %s", bytes_read, filename);
    return true;
}

bool sd_read_chunk(const char* filename, uint32_t offset, uint8_t* buffer, uint32_t size) {
    if (!sd_mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return false;
    }

    char filepath[64];
    snprintf(filepath, sizeof(filepath), "%s/%s", MOUNT_POINT, filename);

    FILE* f = fopen(filepath, "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        return false;
    }

    fseek(f, offset, SEEK_SET);
    size_t bytes_read = fread(buffer, 1, size, f);
    fclose(f);

    if (bytes_read != size) {
        return false;
    }

    return true;
}
