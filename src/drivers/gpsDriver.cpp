#include "drivers/gpsDriver.h"
#include "driver/uart.h"
#include "config/gps_hwconfig.h"

esp_err_t init()
{
  esp_err_t install = uart_driver_install(GpsConfig::uart_num, GpsConfig::RX_buffer,
                                          GpsConfig::TX_buffer, GpsConfig::event_queue_size,
                                          GpsConfig::uart_queue, GpsConfig::interrupt_alloc_flags);
}
