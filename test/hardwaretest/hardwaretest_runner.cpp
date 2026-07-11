#include "hardwaretest_runner.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

void vHardwareTestRunner(void* pvParameters)
{
    run_hardwaretest();
	vTaskDelete(nullptr);
}

extern "C" void app_main()
//{{{
{
  static TaskHandle_t hwtestTaskHandle = nullptr;
  xTaskCreate(vHardwareTestRunner, "HardwaretestRunner", 9000, nullptr, 1, &hwtestTaskHandle);
  vTaskDelete(nullptr);
}
//}}}
