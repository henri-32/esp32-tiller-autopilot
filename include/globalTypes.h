#pragma once
#include <Arduino.h>
#include <cstdint>

enum class NavigationSource { Compass, Gps, Wind };
enum class SteeringDirection { Left, Right };

// hier vllt auf Sekunden ändern und mit Funktion konvertieren
// um speicher zu sparen
struct SystemConfig {
  uint16_t steeringTolerance_deg = 0;
  uint16_t SteeringMinImpulse_ms = 0;
  uint16_t SteeringMaxImpulse_ms = 0;
  uint16_t SteeringCooldown_ms = 0;
  uint16_t minimumTimeBtwObs = 0;
  uint16_t pauseForValidObsAfterImpulse = 0;
};
