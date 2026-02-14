#include "sensors/windModule.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <sys/types.h>
SensorSample<uint16_t> WindModule::read()const{
    SensorSample<uint16_t> sample; 

    const uint16_t raw = 0; /*Hardware lesen*/ 
    if (true /*Plausibilitätscheck*/) {
        sample.value = raw;
        sample.valid = true;
    };
    return sample;
};
