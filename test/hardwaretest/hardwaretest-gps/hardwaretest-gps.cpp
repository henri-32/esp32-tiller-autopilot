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
  full_sentence parser;
  while (1)
  {
    const char* sentence = parser.get_sentence();
    if (sentence != nullptr)
    {
      //printf("%s\n", sentence);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  return 0;
}
