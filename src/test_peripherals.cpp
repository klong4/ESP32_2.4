// Peripheral Test for ST7789 Parallel 8-bit Mode
// Tests: Touch (Software I2C), SD Card (Software SPI), INA219 (Software I2C), ACS712 (ADC)

#include <Arduino.h>
#include <SdFat.h>

// Pin definitions for ST7789 Parallel mode
// Touch Controller (Software I2C)
#define TOUCH_SDA   17
#define TOUCH_SCL   16
#define TOUCH_INT   32
#define TOUCH_RST   8

// SD Card (Software SPI)
#define SD_CS       0
#define SD_MOSI     1
#define SD_MISO     2
#define SD_SCK      7

// INA219 Power Monitor (Software I2C)
#define INA219_SDA  11
#define INA219_SCL  12
#define INA219_ADDR 0x40  // Default I2C address

// ACS712 Current Sensor (ADC)
#define ACS712_PIN  33    // ADC1_CH7

// Software I2C implementation
class SoftI2C {
private:
    uint8_t sda_pin;
    uint8_t scl_pin;
    
    void i2c_start() {
        digitalWrite(sda_pin, HIGH);
        digitalWrite(scl_pin, HIGH);
        delayMicroseconds(5);
        digitalWrite(sda_pin, LOW);
        delayMicroseconds(5);
        digitalWrite(scl_pin, LOW);
        delayMicroseconds(5);
    }
    
    void i2c_stop() {
        digitalWrite(sda_pin, LOW);
        digitalWrite(scl_pin, HIGH);
        delayMicroseconds(5);
        digitalWrite(sda_pin, HIGH);
        delayMicroseconds(5);
    }
    
    bool i2c_write_byte(uint8_t data) {
        for (int i = 7; i >= 0; i--) {
            digitalWrite(scl_pin, LOW);
            digitalWrite(sda_pin, (data >> i) & 1);
            delayMicroseconds(5);
            digitalWrite(scl_pin, HIGH);
            delayMicroseconds(5);
        }
        
        // Wait for ACK
        digitalWrite(scl_pin, LOW);
        pinMode(sda_pin, INPUT_PULLUP);
        delayMicroseconds(5);
        digitalWrite(scl_pin, HIGH);
        delayMicroseconds(5);
        bool ack = digitalRead(sda_pin) == LOW;
        digitalWrite(scl_pin, LOW);
        pinMode(sda_pin, OUTPUT);
        
        return ack;
    }
    
    uint8_t i2c_read_byte(bool ack) {
        uint8_t data = 0;
        pinMode(sda_pin, INPUT_PULLUP);
        
        for (int i = 7; i >= 0; i--) {
            digitalWrite(scl_pin, LOW);
            delayMicroseconds(5);
            digitalWrite(scl_pin, HIGH);
            delayMicroseconds(5);
            if (digitalRead(sda_pin)) {
                data |= (1 << i);
            }
        }
        
        // Send ACK/NACK
        digitalWrite(scl_pin, LOW);
        pinMode(sda_pin, OUTPUT);
        digitalWrite(sda_pin, ack ? LOW : HIGH);
        delayMicroseconds(5);
        digitalWrite(scl_pin, HIGH);
        delayMicroseconds(5);
        digitalWrite(scl_pin, LOW);
        
        return data;
    }
    
public:
    SoftI2C(uint8_t sda, uint8_t scl) : sda_pin(sda), scl_pin(scl) {}
    
    void begin() {
        pinMode(sda_pin, OUTPUT);
        pinMode(scl_pin, OUTPUT);
        digitalWrite(sda_pin, HIGH);
        digitalWrite(scl_pin, HIGH);
    }
    
    bool write(uint8_t addr, uint8_t reg, uint8_t data) {
        i2c_start();
        if (!i2c_write_byte(addr << 1)) return false;
        if (!i2c_write_byte(reg)) return false;
        if (!i2c_write_byte(data)) return false;
        i2c_stop();
        return true;
    }
    
    bool read(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len) {
        i2c_start();
        if (!i2c_write_byte(addr << 1)) return false;
        if (!i2c_write_byte(reg)) return false;
        
        i2c_start();
        if (!i2c_write_byte((addr << 1) | 1)) return false;
        
        for (uint8_t i = 0; i < len; i++) {
            data[i] = i2c_read_byte(i < len - 1);
        }
        i2c_stop();
        return true;
    }
    
    bool scan(uint8_t addr) {
        i2c_start();
        bool found = i2c_write_byte(addr << 1);
        i2c_stop();
        return found;
    }
};

// Software SPI implementation
class SoftSPI {
private:
    uint8_t cs_pin;
    uint8_t mosi_pin;
    uint8_t miso_pin;
    uint8_t sck_pin;
    
public:
    SoftSPI(uint8_t cs, uint8_t mosi, uint8_t miso, uint8_t sck) 
        : cs_pin(cs), mosi_pin(mosi), miso_pin(miso), sck_pin(sck) {}
    
    void begin() {
        pinMode(cs_pin, OUTPUT);
        pinMode(mosi_pin, OUTPUT);
        pinMode(miso_pin, INPUT_PULLUP);
        pinMode(sck_pin, OUTPUT);
        digitalWrite(cs_pin, HIGH);
        digitalWrite(sck_pin, LOW);
    }
    
    uint8_t transfer(uint8_t data) {
        uint8_t result = 0;
        for (int i = 7; i >= 0; i--) {
            digitalWrite(mosi_pin, (data >> i) & 1);
            digitalWrite(sck_pin, HIGH);
            delayMicroseconds(1);
            result |= (digitalRead(miso_pin) << i);
            digitalWrite(sck_pin, LOW);
            delayMicroseconds(1);
        }
        return result;
    }
    
    void select() { digitalWrite(cs_pin, LOW); }
    void deselect() { digitalWrite(cs_pin, HIGH); }
};

// Global objects
SoftI2C touchI2C(TOUCH_SDA, TOUCH_SCL);
SoftI2C ina219I2C(INA219_SDA, INA219_SCL);
SoftSPI sdSPI(SD_CS, SD_MOSI, SD_MISO, SD_SCK);

// Test functions
void test_touch_i2c() {
    Serial.println("\n=== Testing Touch Controller (FT5x26) ===");
    Serial.print("Touch I2C: SDA=");
    Serial.print(TOUCH_SDA);
    Serial.print(", SCL=");
    Serial.println(TOUCH_SCL);
    
    touchI2C.begin();
    
    // Scan for touch controller (common addresses: 0x38, 0x3B)
    Serial.println("Scanning for touch controller...");
    for (uint8_t addr = 0x38; addr <= 0x3B; addr++) {
        if (touchI2C.scan(addr)) {
            Serial.print("✓ Touch controller found at address 0x");
            Serial.println(addr, HEX);
            
            // Try to read chip ID
            uint8_t chipId[2];
            if (touchI2C.read(addr, 0xA3, chipId, 2)) {
                Serial.print("  Chip ID: 0x");
                Serial.println(chipId[0], HEX);
                Serial.print("  Firmware Version: 0x");
                Serial.println(chipId[1], HEX);
            }
            return;
        }
    }
    Serial.println("✗ Touch controller not found");
}

void test_sd_card() {
    Serial.println("\n=== Testing SD Card (Software SPI) ===");
    Serial.print("SD SPI: CS=");
    Serial.print(SD_CS);
    Serial.print(", MOSI=");
    Serial.print(SD_MOSI);
    Serial.print(", MISO=");
    Serial.print(SD_MISO);
    Serial.print(", SCK=");
    Serial.println(SD_SCK);
    
    sdSPI.begin();
    
    // Send CMD0 (GO_IDLE_STATE)
    Serial.println("Sending CMD0 to SD card...");
    sdSPI.select();
    delay(1);
    
    // Send dummy clocks
    for (int i = 0; i < 10; i++) {
        sdSPI.transfer(0xFF);
    }
    
    // Send CMD0
    sdSPI.transfer(0x40 | 0);  // CMD0
    sdSPI.transfer(0x00);
    sdSPI.transfer(0x00);
    sdSPI.transfer(0x00);
    sdSPI.transfer(0x00);
    sdSPI.transfer(0x95);  // CRC for CMD0
    
    // Read response
    uint8_t response = 0xFF;
    for (int i = 0; i < 10; i++) {
        response = sdSPI.transfer(0xFF);
        if (response != 0xFF) break;
    }
    
    sdSPI.deselect();
    sdSPI.transfer(0xFF);  // Extra clock
    
    if (response == 0x01) {
        Serial.println("✓ SD card detected (idle state)");
    } else {
        Serial.print("✗ SD card response: 0x");
        Serial.println(response, HEX);
        Serial.println("  (No card inserted or card not responding)");
    }
}

void test_ina219() {
    Serial.println("\n=== Testing INA219 Power Monitor ===");
    Serial.print("INA219 I2C: SDA=");
    Serial.print(INA219_SDA);
    Serial.print(", SCL=");
    Serial.println(INA219_SCL);
    
    ina219I2C.begin();
    
    // Scan for INA219 (addresses: 0x40-0x4F)
    Serial.println("Scanning for INA219...");
    for (uint8_t addr = 0x40; addr <= 0x4F; addr++) {
        if (ina219I2C.scan(addr)) {
            Serial.print("✓ INA219 found at address 0x");
            Serial.println(addr, HEX);
            
            // Read configuration register (0x00)
            uint8_t config[2];
            if (ina219I2C.read(addr, 0x00, config, 2)) {
                uint16_t configVal = (config[0] << 8) | config[1];
                Serial.print("  Config Register: 0x");
                Serial.println(configVal, HEX);
                
                // Reset and configure
                uint8_t resetCmd[] = {0x80, 0x00};  // Reset bit
                ina219I2C.write(addr, 0x00, resetCmd[0]);
                delay(10);
                
                // Configure: 32V bus, ±320mV shunt, 12-bit continuous
                ina219I2C.write(addr, 0x00, 0x19);  // High byte
                ina219I2C.write(addr, 0x00, 0x9F);  // Low byte
                
                delay(10);
                
                // Read bus voltage (register 0x02)
                uint8_t busVoltage[2];
                if (ina219I2C.read(addr, 0x02, busVoltage, 2)) {
                    uint16_t rawVoltage = (busVoltage[0] << 8) | busVoltage[1];
                    float voltage = (rawVoltage >> 3) * 0.004;  // 4mV per bit
                    Serial.print("  Bus Voltage: ");
                    Serial.print(voltage, 3);
                    Serial.println(" V");
                }
            }
            return;
        }
    }
    Serial.println("✗ INA219 not found");
}

void test_acs712() {
    Serial.println("\n=== Testing ACS712 Current Sensor ===");
    Serial.print("ACS712 ADC: Pin ");
    Serial.print(ACS712_PIN);
    Serial.println(" (ADC1_CH7)");
    
    pinMode(ACS712_PIN, INPUT);
    
    // Read ADC value (average 10 samples)
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(ACS712_PIN);
        delay(10);
    }
    uint16_t adcValue = sum / 10;
    
    // Convert to voltage (Teensy 4.0: 3.3V reference, 10-bit ADC = 1024 levels)
    float voltage = (adcValue / 1023.0) * 3.3;
    
    // ACS712-05B: 185 mV/A, centered at 2.5V
    // ACS712-20A: 100 mV/A, centered at 2.5V
    // ACS712-30A: 66 mV/A, centered at 2.5V
    float current_05B = (voltage - 2.5) / 0.185;
    float current_20A = (voltage - 2.5) / 0.100;
    float current_30A = (voltage - 2.5) / 0.066;
    
    Serial.print("  ADC Value: ");
    Serial.print(adcValue);
    Serial.print(" / 1023");
    Serial.println();
    
    Serial.print("  Voltage: ");
    Serial.print(voltage, 3);
    Serial.println(" V");
    
    Serial.println("  Calculated Current:");
    Serial.print("    ACS712-05B: ");
    Serial.print(current_05B, 2);
    Serial.println(" A");
    Serial.print("    ACS712-20A: ");
    Serial.print(current_20A, 2);
    Serial.println(" A");
    Serial.print("    ACS712-30A: ");
    Serial.print(current_30A, 2);
    Serial.println(" A");
    
    if (voltage < 0.1 || voltage > 3.2) {
        Serial.println("  ⚠ Warning: Voltage out of normal range - check connections");
    } else {
        Serial.println("  ✓ ACS712 reading valid");
    }
}

void setup() {
    Serial.begin(115200);
    delay(3000);
    
    Serial.println("\n\n");
    Serial.println("================================================");
    Serial.println("  ST7789 Parallel 8-bit - Peripheral Test");
    Serial.println("================================================");
    Serial.println();
    Serial.println("Testing all peripherals (Touch, SD, INA219, ACS712)");
    Serial.println();
    
    // Run all tests
    test_touch_i2c();
    delay(500);
    
    test_sd_card();
    delay(500);
    
    test_ina219();
    delay(500);
    
    test_acs712();
    
    Serial.println("\n================================================");
    Serial.println("  All Tests Complete!");
    Serial.println("================================================");
    Serial.println();
    Serial.println("Results summary:");
    Serial.println("- Touch: Check for device found message above");
    Serial.println("- SD Card: Should show idle state (0x01) if card inserted");
    Serial.println("- INA219: Check for address found and voltage reading");
    Serial.println("- ACS712: Voltage should be ~2.5V with no current flow");
    Serial.println();
}

void loop() {
    // Update readings every 2 seconds
    delay(2000);
    
    Serial.println("\n--- Live Sensor Readings ---");
    
    // Read INA219 voltage
    for (uint8_t addr = 0x40; addr <= 0x4F; addr++) {
        if (ina219I2C.scan(addr)) {
            uint8_t busVoltage[2];
            if (ina219I2C.read(addr, 0x02, busVoltage, 2)) {
                uint16_t rawVoltage = (busVoltage[0] << 8) | busVoltage[1];
                float voltage = (rawVoltage >> 3) * 0.004;
                Serial.print("INA219 Bus Voltage: ");
                Serial.print(voltage, 3);
                Serial.println(" V");
            }
            break;
        }
    }
    
    // Read ACS712 current
    uint16_t adcValue = analogRead(ACS712_PIN);
    float voltage = (adcValue / 1023.0) * 3.3;
    float current_20A = (voltage - 2.5) / 0.100;
    
    Serial.print("ACS712 Current (20A): ");
    Serial.print(current_20A, 2);
    Serial.print(" A (");
    Serial.print(voltage, 3);
    Serial.println(" V)");
    
    // Check touch interrupt
    if (digitalRead(TOUCH_INT) == LOW) {
        Serial.println("⚡ Touch detected!");
    }
}
