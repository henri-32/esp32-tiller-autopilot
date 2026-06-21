#pragma once
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "hal/i2c_types.h"
#include <cstdint>

struct I2cConfig
{
  static const i2c_clock_source_t clk_source = I2C_CLK_SRC_DEFAULT;
  static const i2c_port_num_t i2c_port = I2C_NUM_0;
  static const gpio_num_t sda_io_num = GPIO_NUM_21;
  static const gpio_num_t scl_io_num = GPIO_NUM_22;
  static const uint8_t glitch_ignore_cnt = 7;
  static const bool enable_internal_pullup = true;
};

struct CompassConfig
{
  static const i2c_addr_bit_len_t dev_addr_length = I2C_ADDR_BIT_LEN_7;
  // Data Sheet Compass QML5883P
  // https://www.qstcorp.com/upload/pdf/202202/%EF%BC%88%E5%B7%B2%E4%BC%A0%EF%BC%8913-52-19%20QMC5883P%20Datasheet%20Rev.C(1).pdf
  static const uint16_t device_adress = 0x2c;
  static const uint32_t scl_speed_hz = 10000;
  static const uint16_t write_timeout_ms = 10;
  static const uint8_t control_reg1 = 0x0a;
  // control_val represents configuration of the hardwaremodule from the data sheet.
  static const uint8_t control_val = 0b01100011;
  // Data Registers
  static const uint8_t compassdata_register_start = 0x01;
  static const uint8_t compassdata_register_bytes = 6; 
};
