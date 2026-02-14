#pragma once 


/* Holt sich Geschwindigkeit durchs Wasser von NASA Clipper über 
NMEA183 Protokoll
Weil das Ausgewertet werden muss, ist es ein eigenes Modul*/

#include "types/globalTypes.h"
class NMEA183BUS {
    public:
// Contract:
// Purpose: Read speed through water (STW) via NMEA.
// Inputs: NMEA bus messages.
// Outputs/Side-effects: returns SensorSample; no side-effects.
NMEA183BUS() = default; 

SensorSample<float> readSTW() const;
};
