#pragma once
#include "driver/uart.h"
#include <cstdint>

struct GpsConfig 
{
  static constexpr uart_port_t uart_num = UART_NUM_2; 
  static constexpr uint16_t RX_buffer = 2048; 
  static constexpr uint16_t TX_buffer = 0; 
  static constexpr int event_queue_size = 0; 
  static constexpr QueueHandle_t* uart_queue = nullptr; 
  static constexpr int interrupt_alloc_flags = 0; 

}; 
