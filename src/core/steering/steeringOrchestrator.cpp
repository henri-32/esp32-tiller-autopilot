#include "core/steering/steeringOrchestrator.h"
#include "core/steering/csc/csc.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>

SteeringOrchestrator::SteeringOrchestrator(CoreSteeringController &csc,
                                           ImpulseFilter &filter,
                                           PWMController &pwm)
    :m_csc(csc), m_impulsefilter(filter),
      m_pwm(pwm){};

void SteeringOrchestrator::tick(NavigationSensors::NavigationSnapshot snapshot,
                                uint32_t loopTimestamp) {


  m_csc.currentHDG(snapshot.compass_hdg_dg.value);

  auto intentOpt = m_csc.tick(loopTimestamp);

  if (intentOpt) {
    SteeringIntent intent = *intentOpt;
    auto cmd = m_impulsefilter.apply(intent, snapshot);
    m_pwm.command(cmd);
  }
};
