#include "compassModuleHardwareTest.h"

extern "C" void app_main() 
{
  gpio_reset_pin(COMPASS_GPIO);
  gpio_set_direction(
      COMPASS_GPIO,
      GPIO_MODE_OUTPUT); // TODO Muss auf Input geändert werden, nur zum Build validieren

  while (1)
  {
    gpio_set_level(COMPASS_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(500)); 
  }
}
