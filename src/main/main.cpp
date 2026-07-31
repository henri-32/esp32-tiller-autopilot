#include "core/inputHandler.h"
#include "core/queueServer.h"
#include "core/steering/csc/csc.h"
#include "core/telemetryHub.h"
#include "drivers/gpsDriver.h"
#include "drivers/uartDriver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logging/logger.h"

extern "C" void app_main()
{

  static UartDriver uartDriver{};
  static QueueServer qServer{};
  static GpsDriver gpsDriver{&qServer, &uartDriver};

  static TelemetryHub tHub{&qServer};

  static Logger logger{&qServer, &uartDriver};
  static InputHandler inputHandler{&qServer, &uartDriver};

  static CoreSteeringController csc{&qServer};

  constexpr TickType_t initialization_retry_delay = pdMS_TO_TICKS(1000);

  // Initialize dependencies in order. Once a component succeeds, only the next
  // component is retried, so already running tasks and installed drivers are
  // never initialized a second time.
  while (qServer.init() != pdPASS)
  {
    vTaskDelay(initialization_retry_delay);
  }

  while (uartDriver.init() != ESP_OK)
  {
    vTaskDelay(initialization_retry_delay);
  }

  while (gpsDriver.init() != ESP_OK)
  {
    vTaskDelay(initialization_retry_delay);
  }

  while (tHub.init() != pdPASS)
  {
    vTaskDelay(initialization_retry_delay);
  }

  while (logger.init() != pdPASS)
  {
    vTaskDelay(initialization_retry_delay);
  }

  while (inputHandler.init() != ESP_OK)
  {
    vTaskDelay(initialization_retry_delay);
  }

  vTaskDelete(nullptr);
}
