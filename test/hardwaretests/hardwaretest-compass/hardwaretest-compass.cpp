#include "driver/gpio.h"
#include "drivers/compassModule.h"
#include "drivers/i2c_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

// GPIO 21 for sda and GPIO 22 for scl required
#define GREEN_LED_GPIO GPIO_NUM_16
#define RED_LED_GPIO GPIO_NUM_17

static bool askedForRun = false;

int run_test()
{
  gpio_reset_pin(RED_LED_GPIO);
  gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);

  const char* TAG = "COMPASSTEST";
  I2cDriver i2c_driver{};
  esp_err_t bus_init = i2c_driver.init();

  static CompassModule compassModule{i2c_driver.get_bus_handler()};
  esp_err_t compass_init = compassModule.init();

  if (bus_init != ESP_OK || compass_init != ESP_OK)
  {
    ESP_LOGE(TAG, "Initialization of compassModule failed");
    return 1;
  }

  return 0;
}

extern "C" void app_main()
//{{{
{
  const char* TAG = "MAIN";

  while (1)
  {
    if (askedForRun == false)
    {
      printf("Do you want to run the hardwaretest-compass? \n [y/n] \n");
      askedForRun = true;
    };

    int c = getchar();

    if (c == 'y' || c == 'Y')
    {
      run_test();
    }
    if (c == 'n' || c == 'N')
    {
      printf("No tests were started");
      return;
    }
    c = ' ';
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
//}}}
