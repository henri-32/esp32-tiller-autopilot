#include "sensors/compassModule.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <sys/types.h>
SensorSample<uint16_t> CompassModule::read()const{
    SensorSample<uint16_t> sample; 

    const uint16_t raw = 0; /*Hardware lesen*/ 
    if (true /*Plausibilitätscheck*/) {
        sample.value = raw;
        sample.valid = true;
    };
    return sample;
};
