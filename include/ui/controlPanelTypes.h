#pragma once 

#include "types/globalTypes.h"
#include <cstdint>

struct PanelIntent {
    bool steeringEngaged;
    uint16_t generalTarget; 

    enum class SystemMode {
        Normal, 
        Developer, 
        Safe, 
    };
    NavigationSource activeSource;
};