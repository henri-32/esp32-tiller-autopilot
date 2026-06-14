#include "driver/gpio.h"
#include "drivers/compassModule.h"
#include "drivers/i2c_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

// GPIO 21 for sda and GPIO 22 for scl needed
#define COMPASS_GPIO GPIO_NUM_16
#define RED_LED_GPIO GPIO_NUM_17

static bool askedForRun = false;

int init_test()
{
  I2cDriver i2c_driver{};
  esp_err_t bus_init = i2c_driver.init_bus();

  CompassModule compassModule{i2c_driver.get_bus_handler()};
  esp_err_t compass_init = compassModule.init();

  gpio_reset_pin(COMPASS_GPIO);
  gpio_set_direction(
      COMPASS_GPIO,
      GPIO_MODE_OUTPUT); // TODO Muss auf Input geändert werden, nur zum Build validieren
  gpio_reset_pin(RED_LED_GPIO);
  gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);

  if (bus_init != ESP_OK || compass_init != ESP_OK)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

int run_test()
{

  while (1)
  {
    gpio_set_level(RED_LED_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    return 0;
  }
}

extern "C" void app_main()
{

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
      if (init_test() == 0)
      {
        printf("init_test() succeded...\n");
        fflush(stdout);
        run_test();
      }
      else
      {
        printf("init_test() failed");
        return;
      }
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
