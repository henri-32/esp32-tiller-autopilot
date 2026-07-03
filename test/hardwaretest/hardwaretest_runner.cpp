#include "hardwaretest_runner.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

extern "C" void app_main()
//{{{
{
  const char* TAG = "MAIN";

  while (1)
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
//}}}


