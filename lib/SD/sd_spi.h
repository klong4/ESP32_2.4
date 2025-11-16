#ifndef SD_SPI_H
#define SD_SPI_H

#include <stdint.h>
#include <stdbool.h>

// Initialize SD card on SPI bus
bool sd_init(int cs_pin, int mosi_pin, int miso_pin, int clk_pin);

// Mount SD card filesystem
bool sd_mount(void);

// Unmount SD card filesystem
void sd_unmount(void);

// Load image from SD card (240x320 RGB565 format)
// Returns true if successful
bool sd_load_image(const char* filename, uint16_t* buffer, uint32_t max_size);

// Load partial image data (for streaming)
bool sd_read_chunk(const char* filename, uint32_t offset, uint8_t* buffer, uint32_t size);

#endif // SD_SPI_H
