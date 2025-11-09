#pragma once
#include "driver/i2c.h"
#include "esp_err.h"
#include <cstdint>

class PCA9685 {
public:
    PCA9685(uint8_t i2c_addr = 0x40);

    // Methods
    esp_err_t init_pca(i2c_port_t i2c_port = I2C_NUM_0, gpio_num_t sda = GPIO_NUM_21, gpio_num_t scl = GPIO_NUM_22, uint32_t freq_hz = 100000);
    void set_PWM(uint8_t channel, uint16_t on, uint16_t off);

private:
    // Attributes
    uint8_t _i2c_addr;
    i2c_port_t _i2c_port;

    // Methods
    void set_PWM_freq(float freq);
    esp_err_t write_register(uint8_t reg, uint8_t value);
    uint8_t read_register(uint8_t reg);
};
