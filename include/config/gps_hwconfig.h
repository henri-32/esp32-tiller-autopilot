#pragma once
#include "driver/uart.h"
#include "driver/gpio.h"

#include <cstdint>

struct GpsConfig 
{
  static constexpr gpio_num_t RX_GPIO = GPIO_NUM_16;
  static constexpr gpio_num_t TX_GPIO = GPIO_NUM_17;
  static constexpr gpio_num_t GPIO = RX_GPIO;
  static constexpr uart_port_t uart_num = UART_NUM_2; 
  static constexpr uint16_t RX_buffer = 2048; 
  static constexpr uint16_t TX_buffer = 256; 
  static constexpr int event_queue_size = 0; 
  static constexpr QueueHandle_t* uart_queue = nullptr; 
  static constexpr int interrupt_alloc_flags = 0; 

}; 
