#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include <cstddef>
#include <cstdint>

enum class UartInterface
{
  GATEWAY = 1,
  GPS = 2
};

class UartDriver
{
public:
  UartDriver() = default;

  esp_err_t init();
  int write(UartInterface channel, const void* src, size_t size);
  int read(UartInterface channel, void* const dest, uint16_t dest_buf_len,
           TickType_t timeout) const;

private:
  static constexpr size_t gateway_encoded_buffer_size_ = 1024;
  mutable uint8_t gateway_encoded_buffer_[gateway_encoded_buffer_size_]{};
  mutable size_t gateway_encoded_length_ = 0;
  mutable bool discard_gateway_frame_ = false;
};
