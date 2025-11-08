#include "PCA9685.hpp"
#include "esp_log.h"
#include <cmath>

#define MODE1_REG 0x00
#define PRESCALE_REG 0xFE
#define LED0_ON_L 0x06

PCA9685::PCA9685(uint8_t i2c_addr) 
    : _i2c_addr(i2c_addr), _i2c_port(I2C_NUM_0) {}


esp_err_t PCA9685::init(i2c_port_t i2c_port, gpio_num_t sda, gpio_num_t scl, uint32_t freq_hz) {
    _i2c_port = i2c_port;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = freq_hz}
    };
    ESP_ERROR_CHECK(i2c_param_config(_i2c_port, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(_i2c_port, conf.mode, 0, 0, 0));

    writeRegister(MODE1_REG, 0x00);
    setPWMFreq(50.0f);
    ESP_LOGI("PCA9685", "Initialized PCA9685 at 50 Hz");
    return ESP_OK;
}

void PCA9685::setPWM(uint8_t channel, uint16_t on, uint16_t off) {
    uint8_t data[5] = {uint8_t(LED0_ON_L + 4 * channel),
                       uint8_t(on & 0xFF), uint8_t(on >> 8),
                       uint8_t(off & 0xFF), uint8_t(off >> 8)};
    i2c_master_write_to_device(_i2c_port, _i2c_addr, data, 5, pdMS_TO_TICKS(100));
}


////// Private methods //////

void PCA9685::setPWMFreq(float freq) {
    float prescaleval = 25000000.0 / (4096.0 * freq) - 1.0;
    uint8_t prescale = static_cast<uint8_t>(floor(prescaleval + 0.5));

    uint8_t oldmode = readRegister(MODE1_REG);
    uint8_t newmode = (oldmode & 0x7F) | 0x10;
    writeRegister(MODE1_REG, newmode);
    writeRegister(PRESCALE_REG, prescale);
    writeRegister(MODE1_REG, oldmode);
    vTaskDelay(pdMS_TO_TICKS(5));
    writeRegister(MODE1_REG, oldmode | 0xA1);
}

esp_err_t PCA9685::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_master_write_to_device(_i2c_port, _i2c_addr, data, 2, pdMS_TO_TICKS(100));
}


uint8_t PCA9685::readRegister(uint8_t reg) {
    uint8_t value;
    i2c_master_write_read_device(_i2c_port, _i2c_addr, &reg, 1, &value, 1, pdMS_TO_TICKS(100));
    return value;
}

