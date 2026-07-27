#pragma once
#include <cstddef>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

enum class UartInterface
{
  USB_INTERFACE = 1,
};

class UartDriver
{
public:
  UartDriver() = default;

  esp_err_t init();
  int write(UartInterface channel, const void* src, size_t size);
  int read(UartInterface channel, void* const dest, uint16_t dest_buf_len, TickType_t timeout);

private:
};
