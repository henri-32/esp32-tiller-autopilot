#pragma once

#include "core/config.h"
#include <cstdint>

class Deadband {
public:
    Deadband(SteeringController_Config& config);

bool errorSignificant(int16_t error);

private:
SteeringController_Config& m_config;

};
