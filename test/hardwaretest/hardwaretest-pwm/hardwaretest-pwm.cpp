#include "driver/ledc.h"
#include "drivers/pwmDriver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void vHardwaretestPwm1(void* pvParameters)
{
  while (true)
  {
    for (int i = 0; i < 255; i++)
    {
      ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
      ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
      vTaskDelay(pdMS_TO_TICKS(10));
    }

    for (int i = 255; i > 0; i--)
    {

      ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
      ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}
void vHardwaretestPwm2(void* pvParameters)
{
  while (true)
  {
    for (int i = 255; i > 0; i--)
    {

      ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, i);
      ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
      vTaskDelay(pdMS_TO_TICKS(10));
    }

    for (int i = 0; i < 255; i++)
    {
      ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, i);
      ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}

int run_hardwaretest()
{
  static PwmDriver driver{};

  if (driver.init() != ESP_OK)
  {
    return 1;
  }

  xTaskCreate(vHardwaretestPwm1, "", 2000, nullptr, 3, nullptr);
  xTaskCreate(vHardwaretestPwm2, "", 2000, nullptr, 3, nullptr);

  return 0;
}
