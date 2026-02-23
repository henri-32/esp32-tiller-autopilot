#include "stm32f4xx_hal.h" // IWYU pragma: keep
#include "platform/platform.h"
#include "core/systemController.h"
#include <sys/_intsup.h>

SystemController systemController;

int main()
{
    // --------------------------------------------------------
    // Platform init (HAL, clock, SysTick base setup)
    // --------------------------------------------------------
    Platform_Init();

    // --------------------------------------------------------
    // Timer sanity check (SysTick must advance)
    // --------------------------------------------------------
    const uint32_t t0 = HAL_GetTick();
    HAL_Delay(2); // uses SysTick
    const uint32_t t1 = HAL_GetTick();

    if (t1 <= t0)
    {
        FatalError("SysTick not running");
    }

    //---------------------------------------------------------
    //HardwareInit 
    //---------------------------------------------------------

    //TODO
    //Meine Hardware
    
    // --------------------------------------------------------
    // Main loop
    // --------------------------------------------------------
    while (true)
    {
      uint32_t loopTimestamp = HAL_GetTick();
        systemController.tick(loopTimestamp);
    }
}
