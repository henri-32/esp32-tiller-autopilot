#pragma once 


/* Holt sich Geschwindigkeit durchs Wasser von NASA Clipper über 
NMEA183 Protokoll
Weil das Ausgewertet werden muss, ist es ein eigenes Modul*/

class NMEA183BUS {
    public:
NMEA183BUS() = default; 

float getHullSpeed() const;
};