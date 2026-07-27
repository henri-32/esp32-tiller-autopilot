#include "core/queueServer.h"
#include "core/telemetryHub.h"
#include "core/inputHandler.h"
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

  vTaskDelete(nullptr);
}
