#pragma once 
#include <cstdint> 

enum class SteeringDirection { portside, starbord };

struct SteeringIntent {
  SteeringDirection dir;
  uint8_t abstractImpulse_0_100;
};

struct PWMIntent {
  SteeringDirection dir;
  uint16_t filteredAbstractImpulse_0_100;
  uint16_t duration;


};

