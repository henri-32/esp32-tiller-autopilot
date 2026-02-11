#include "core/steering/steeringOrchestrator.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>

SteeringOrchestrator::SteeringOrchestrator(SourceEvaluator &eval,
                                           CoreSteeringController &csc,
                                           ImpulseFilter &filter,
                                           PWMController &pwm)
    : m_sourceEvaluator(eval), m_csc(csc), m_impulsefilter(filter),
      m_pwm(pwm) {};

void SteeringOrchestrator::tick(NavigationSensors::NavigationSnapshot snapshot,
                                uint32_t loopTimestamp) {
  m_sourceEvaluator.tick(snapshot, loopTimestamp);

  auto intentOpt = m_csc.tick(loopTimestamp);
  if (intentOpt) {
    SteeringIntent intent = *intentOpt;
    auto cmd = m_impulsefilter.apply(intent, snapshot);
    m_pwm.command(cmd);
  }
};