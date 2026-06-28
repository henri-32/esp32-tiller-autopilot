#pragma once
#include "driver/i2c_master.h"
#include "driver/uart.h"
#include "hal/i2c_types.h"
#include <cstdint>

struct I2cConfig
{
  static constexpr i2c_clock_source_t clk_source = I2C_CLK_SRC_DEFAULT;
  static constexpr i2c_port_num_t i2c_port = I2C_NUM_0;
  static constexpr gpio_num_t sda_io_num = GPIO_NUM_21;
  static constexpr gpio_num_t scl_io_num = GPIO_NUM_22;
  static constexpr uint8_t glitch_ignore_cnt = 7;
  static constexpr bool enable_internal_pullup = true;
};

struct CompassConfig
{
  static constexpr i2c_addr_bit_len_t dev_addr_length = I2C_ADDR_BIT_LEN_7;
  // Data Sheet Compass QML5883P
  // https://www.qstcorp.com/upload/pdf/202202/%EF%BC%88%E5%B7%B2%E4%BC%A0%EF%BC%8913-52-19%20QMC5883P%20Datasheet%20Rev.C(1).pdf
  static constexpr uint16_t device_adress = 0x2c;
  static constexpr uint32_t scl_speed_hz = 10000;
  static constexpr uint16_t write_timeout_ms = 10;
  static constexpr uint8_t control_reg1 = 0x0a;
  // control_val represents configuration of the hardwaremodule from the data sheet.
  static constexpr uint8_t control_val = 0b01100011;
  // Data Registers
  static constexpr uint8_t compass_data_reg_start_addr = 0x01;
  static constexpr uint8_t compass_data_reg_byte_len = 6; 
  static constexpr uint8_t compass_status_reg = 0x09;
};

struct GpsConfig 
{
  static constexpr uart_port_t uart_num = UART_NUM_2; 
  static constexpr uint16_t RX_buffer = 2048; 
  static constexpr uint16_t TX_buffer = 0; 
  static constexpr int event_queue_size = 0; 
  static constexpr QueueHandle_t* uart_queue = nullptr; 
  static constexpr int interrupt_alloc_flags = 0; 

}; 
