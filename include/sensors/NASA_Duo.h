#pragma once

/* Holt sich Geschwindigkeit durchs Wasser von NASA Clipper über
NMEA183 Protokoll
Weil das Ausgewertet werden muss, ist es ein eigenes Modul*/

#include "types/globalTypes.h"
class NASA_Duo {
public:
  // Contract:
  // Purpose: Read speed through water (STW) via NMEA.
  // Inputs: NMEA bus messages.
  // Outputs/Side-effects: returns SensorSample; no side-effects.
  NASA_Duo() = default;

  SensorSample<float> readSTW() const;
};
