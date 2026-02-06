#include <Arduino.h>

class CompassModule {
public:
  uint16_t readHeading() const;
};