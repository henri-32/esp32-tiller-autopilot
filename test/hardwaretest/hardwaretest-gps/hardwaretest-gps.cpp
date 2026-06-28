#include "cstdio"
#include "driver/gpio.h"
#include "drivers/gpsDriver.h"
#include "drivers/i2c_driver.h"
#include "esp_log.h"

int run_hardwaretest()
{
  printf("Implementation 'run_hardwaretest()' from: %s\n", static_cast<const char*>(__FILE__));
  printf("gps-hardwaretest running\n");
  return 0;
}
