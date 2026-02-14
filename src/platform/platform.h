#pragma once



// ------------------------------------------------------------
// Platform lifecycle
// ------------------------------------------------------------

// Initializes HAL, clock and mandatory system infrastructure.
// On failure: does not return.
void Platform_Init();

// Non-recoverable system failure.
// Never returns.
[[noreturn]] void FatalError(const char* reason);

// Configures system clocks. Must be called after HAL_Init().
void SystemClock_Config();
