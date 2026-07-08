#pragma once
#include "driver/uart.h"
#include "driver/gpio.h"
#include "types/loggingTypes.h"

#include <cstdint>

struct GpsConfig 
{
  static constexpr gpio_num_t RX_GPIO = GPIO_NUM_16;
  static constexpr gpio_num_t TX_GPIO = GPIO_NUM_17;
  static constexpr gpio_num_t GPIO = RX_GPIO;
  static constexpr uart_port_t uart_num = UART_NUM_2; 
  static constexpr uart_word_length_t data_bits = UART_DATA_8_BITS;
  static constexpr uart_parity_t parity = UART_PARITY_DISABLE; 
  static constexpr uart_stop_bits_t stop_bits = UART_STOP_BITS_1;
  static constexpr uart_hw_flowcontrol_t flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  static constexpr uint16_t RX_buffer = 2048; 
  static constexpr uint16_t TX_buffer = 256; 
  static constexpr uint16_t max_sentence_len = NmeaConfig::max_sentence_len;
  static constexpr int event_queue_size = 0; 
  static constexpr QueueHandle_t* uart_queue = nullptr; 
  static constexpr int interrupt_alloc_flags = 0; 
  static constexpr uint16_t task_period = 2000;
  static constexpr uint16_t baud_rate = 9600;

}; 
