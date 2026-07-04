#include "driver/uart.h"
#include "drivers/gpsDriver.h"
#include "drivers/i2c_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio>

int run_hardwaretest()
{
  GpsDriver driver; 
  driver.init();
  while (1)
  {
  }
  return 0;
}
