#include "core/queueServer.h"
#include "driver/uart.h"
#include "drivers/gpsDriver.h"
#include "drivers/i2c_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio>

void vPrintGpsTask(void* pvParameters)
{
  task_context* context = static_cast<task_context*>(pvParameters);

  while (true)
  {
    Telemetry<GpsData> gpsTel{};

    xQueueReceive(context->queueBundle.data, &gpsTel.data, pdMS_TO_TICKS(2000));
    xQueueReceive(context->queueBundle.performance, &gpsTel.performance, pdMS_TO_TICKS(2000));

    printf("SOG: %f / expected: 0.0\nfixQuality: %d / expected: 0\nFreeTaskStackGPS: %d \n",
           gpsTel.data.speed_kts, gpsTel.data.fixQuality, gpsTel.performance.free_task_stack);
  }
};

int run_hardwaretest()
{
  static QueueServer qServer;
  static GpsDriver driver;

  BaseType_t qs = qServer.init();

  esp_err_t di = driver.init(qServer.get_gps_bundle());

  BaseType_t tc;

  if (driver.context_ != nullptr)
  {
    tc = xTaskCreate(vPrintGpsTask, "", 10000, driver.context_, 1, nullptr);
  }

  if (di == ESP_OK && qs == pdPASS && tc == pdPASS)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
