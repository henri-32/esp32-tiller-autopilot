#include "driver/gpio.h"
#include "drivers/compassModule.h"
#include "drivers/i2c_driver.h"
#include "esp_log.h"

// GPIO 21 for sda and GPIO 22 for scl required
#define GREEN_LED_GPIO GPIO_NUM_16
#define RED_LED_GPIO GPIO_NUM_17

int run_hardwaretest()
{
  gpio_reset_pin(RED_LED_GPIO);
  gpio_set_direction(RED_LED_GPIO, GPIO_MODE_OUTPUT);

  const char* TAG = "COMPASSTEST";
  I2cDriver i2c_driver{};
  esp_err_t bus_init = i2c_driver.init();

  static CompassModule compassModule{i2c_driver.get_master_bus_handle()};
  esp_err_t compass_init = compassModule.init();

  if (bus_init != ESP_OK || compass_init != ESP_OK)
  {
    ESP_LOGE(TAG, "Initialization of compassModule failed");
    return 1;
  }

  while (true)
  {
  //  printf("x: %d \ny: %d \nz: %d \n", compassModule.read_raw().x, compassModule.read_raw().y,
   //        compassModule.read_raw().z);
  printf("Heading: %d \n", compassModule.calc_heading_from_raw(compassModule.read_raw()));  
}
  return 0;
}

