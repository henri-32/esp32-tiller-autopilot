#pragma once
#include <cstdint>

enum class DiagnosticEvent : uint8_t {
    Degraded,
    Lost,
    LostWithFallback,
    ErrorHandled
};

enum class FunctionalCapability : uint8_t {
    COMPASS,
    GPS,
    WIND,
    HullSpeed,
    PWM,
    COUNT
};

enum class CapabilityState : uint8_t {
    OK,
    Degraded,
    Lost
};
