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
  static GpsDriver gpsDriver{&qServer};

  static TelemetryHub tHub{&qServer};

  static Logger logger{&qServer, &uartDriver};
  static InputHandler inputHandler{&qServer, &uartDriver};

  static CoreSteeringController csc{&qServer};

  while (true)
  {
    // Trys to initialize the system until success
    if (uartDriver.init() != ESP_OK)
    {
      continue;
    }

    if (logger.init() != pdPASS)
    {
      continue;
    }

    if (gpsDriver.init() != ESP_OK)
    {
      continue;
    }

    if (inputHandler.init() != pdPASS)
    {
      continue;
    }

	break;
  }

  vTaskDelete(nullptr);
}
