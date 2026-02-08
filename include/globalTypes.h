#pragma once
#include <cstdint>

enum class NavigationSource { Compass, Gps, Wind };
enum class SteeringDirection { Left, Right };

// Siehe README CSC_Config !!!
struct CSC_Config {

  //HDG steering mechanics
  uint8_t steeringTolerance_deg = 10;
  unsigned long SteeringMinImpulse_ms = 100;
  unsigned long SteeringMaxImpulse_ms = 1000;
  unsigned long SteeringCooldown_ms = 3000;
  unsigned long minimumTimeBtwObs_ms = 1000;
  static constexpr uint8_t observationBufferSize = 70;
  unsigned long pauseForValidObsAfterImpulse_ms = 3000;

  // Source Filters 
  float minimumGPS_SpeedForGPS_Use = 1.0;
};
