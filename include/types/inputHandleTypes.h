#pragma once

#include <cstdint>


// Steering from the Laptop to the autopilot
struct InputHandleData_t
{
  bool steering_engaged = false;
  int16_t target_course = 0;
};
