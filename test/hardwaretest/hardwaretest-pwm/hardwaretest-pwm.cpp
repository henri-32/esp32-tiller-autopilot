#include "driver/ledc.h"
#include "drivers/pwmDriver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Testet successfully against Commit:
 *03fe5d2d5f291db53630c11c8cb6db6808e437fd
 */

void vRunHardwaretestTask(void* pvParameters)
{
  PwmDriver driver = *static_cast<PwmDriver*>(pvParameters);
  PWMIntent intent{};

  /*Different abstract impulses/durations/directions over the intended API of the PwmDriver are
   * testet. With LEDs on the GPIOS of the PwmDriver the steering impulse to the MOSFET H-Bridge can
   * be watched.
   */
  for (int i = 0; i < 2; i++)
  {
    intent.dir = SteeringDirection::portside;
    intent.filteredAbstractImpulse_0_100 = 100;
    intent.duration = 1000;

    driver.command(intent);

    vTaskDelay(pdMS_TO_TICKS(1000));

    intent.dir = SteeringDirection::starbord;
    intent.filteredAbstractImpulse_0_100 = 100;
    intent.duration = 1000;

    driver.command(intent);

    intent.dir = SteeringDirection::portside;
    intent.filteredAbstractImpulse_0_100 = 10;
    intent.duration = 3000;

    driver.command(intent);

    intent.dir = SteeringDirection::starbord;
    intent.filteredAbstractImpulse_0_100 = 10;
    intent.duration = 3000;

    driver.command(intent);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

   

  vTaskDelete(nullptr);
}

int run_hardwaretest()
{
  static PwmDriver driver{};

  if (driver.init() != ESP_OK)
  {
    return 1;
  }

  xTaskCreate(vRunHardwaretestTask, "", 5000, &driver, 2, nullptr);

  return 0;
}
