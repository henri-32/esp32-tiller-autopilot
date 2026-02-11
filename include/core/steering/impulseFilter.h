#pragma once
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"


class ImpulseFilter {
public:

  ImpulseFilter(SteeringController_Config &config);

  PWMCommand apply(SteeringIntent intent, NavigationSensors::NavigationSnapshot snapshot);

private:
  SteeringController_Config &m_config;
};