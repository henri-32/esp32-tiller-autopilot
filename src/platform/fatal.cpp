#include "platform/platform.h"
#include "stm32f4xx_hal.h" // IWYU pragma: keep

[[noreturn]] void FatalError(const char* /*reason*/)
{
    __disable_irq();

    // Hier später möglich:
    // - Breakpoint
    // - Watchdog absichtlich auslösen
    // - LED-Blink-Code
    // - Backup-Register beschreiben

    while (true)
    {
        // System bewusst eingefroren
    }
}
