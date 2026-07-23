#pragma once
#include "esp_err.h"

enum class uartInterface
{
  USB_OUT = 1,
};

class UartDriver
{
public:
  UartDriver() = default;

  esp_err_t init();
  int write(uartInterface channel, const void* src, size_t size);

private:
};
