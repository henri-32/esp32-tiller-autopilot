#pragma once

#include <cstdint>


// Steering from the Laptop to the autopilot
struct InputHandleData_t
{
  bool engage = false;
  uint16_t target_course = 0;
};
