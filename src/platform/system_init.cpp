#include "stm32f4xx_hal.h" // IWYU pragma: keep
#include "platform/platform.h"

void Platform_Init()
{
    if (HAL_Init() != HAL_OK)
    {
        FatalError("HAL_Init failed");
    }

    SystemClock_Config();
}
