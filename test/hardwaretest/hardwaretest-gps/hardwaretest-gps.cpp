#include "driver/uart.h"
#include "drivers/gpsDriver.h"
#include "drivers/i2c_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio>

void vPrintGpsTask(void* pvParameters)
{
  gps::task_context* context = static_cast<gps::task_context*>(pvParameters);

  while (true)
  {
    gps::data data{};
    xQueueReceive(context->gpsQueue, &data, pdMS_TO_TICKS(2000));

    printf("SOG: %f / expected: 0.0\nfixQuality: %d / expected: 0\n", data.speed_kts,
           data.fixQuality);
  }
};

int run_hardwaretest()
{
  static GpsDriver driver;
  esp_err_t di = driver.init();

  BaseType_t tc = xTaskCreate(vPrintGpsTask, "", 10000, driver.context_, 1, nullptr);

  if (di == ESP_OK && tc == pdPASS)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
