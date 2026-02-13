#include "core/steering/csc/headingErrorCalculator.h"

int16_t HeadingErrorCalculator::getCurrentError(uint16_t current,
                                                  uint16_t target) {

  int16_t diff = target - current;

  diff += 360;
  diff += 180;
  diff %= 360;
  diff -= 180;

  return diff; 
};