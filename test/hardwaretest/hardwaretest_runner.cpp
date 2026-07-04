#include "hardwaretest_runner.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

void vHardwareTestRunner(void* pvParameters)
{
  const char* TAG = "HardwareTestRunner";
  while (true)
  {
    if (!testRan && !askedForTest)
    {
      printf("Do you want to run the hardwaretest? \n [y/n] \n");
      askedForTest = true;
    };

    int c = getchar();

    if (c == 'y' || c == 'Y')
    {
      test_return_val = run_hardwaretest();
      testRan = true;
    }
    if (c == 'n' || c == 'N')
    {
      printf("No tests were started");
      return;
    }
    c = ' ';
    if (testRan)
    {
      if (test_return_val == 0)
      {
        printf("Testrun finished\n");
      }
      else
      {
        printf("Testrun failed\n");
      }
      return;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

extern "C" void app_main()
//{{{
{
  static TaskHandle_t hwtestTaskHandle = nullptr;
  xTaskCreate(vHardwareTestRunner,
              "Decicion to run hardwaretest/return of Hardwaretest if automated", 3000, nullptr, 1,
              &hwtestTaskHandle);
  while (true)
  {
    uint16_t minimum_free = uxTaskGetStackHighWaterMark(hwtestTaskHandle);
    printf("================ HARDWARETESTRUNNER MINIMUM FREE %d BYTES ================\n", minimum_free);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
//}}}
