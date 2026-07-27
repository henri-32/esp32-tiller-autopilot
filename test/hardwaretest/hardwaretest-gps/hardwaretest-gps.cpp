#include "core/telemetryHub.h"
#include "core/inputHandler/inputHandler.h"
#include "core/queueServer.h"
#include "driver/uart.h"
#include "drivers/gpsDriver.h"
#include "drivers/i2c_driver.h"
#include "drivers/uartDriver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logging/logger.h"

#include <cstdio>

void vPrintGpsTask(void* pvParameters)
{
  task_context* context = static_cast<task_context*>(pvParameters);

  while (true)
  {
    Telemetry<gpsDriverData_t> gpsTel{};

    xQueueReceive(context->queueBundle.data, &gpsTel.data, pdMS_TO_TICKS(2000));
    xQueueReceive(context->queueBundle.runtime_log, &gpsTel.performance, pdMS_TO_TICKS(2000));

    //printf("SOG: %f / expected: 0.0\nfixQuality: %d / expected: 0\nFreeTaskStackGPS: %d \n",
     //      gpsTel.data.speed_kts, gpsTel.data.fixQuality, gpsTel.performance.free_task_stack);
  }
};

int run_hardwaretest()
{
  static QueueServer qServer;
  static GpsDriver gpsDriver{&qServer};
  static TelemetryHub telemetryHub{&qServer};
  static UartDriver uartDriver{};
  static Logger logger{&qServer, &uartDriver};
  static InputHandler inputHandler{&qServer, &uartDriver};

  BaseType_t qs = qServer.init();

  esp_err_t di;
  esp_err_t ui;
  BaseType_t li;
  BaseType_t si;

  if (qs == pdPASS)
  {

    di = gpsDriver.init();
	ui = uartDriver.init();
    si = telemetryHub.init();

    li = logger.init();
  }

  BaseType_t tc;

  if (gpsDriver.context_ != nullptr)
  {
     //tc = xTaskCreate(vPrintGpsTask, "", 10000, driver.context_, 1, nullptr);
  }

  if (di == ESP_OK && qs == pdPASS && tc == pdPASS && li == pdPASS)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
