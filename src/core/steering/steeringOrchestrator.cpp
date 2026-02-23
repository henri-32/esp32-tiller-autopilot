#include "core/steering/steeringOrchestrator.h"
#include "core/steering/csc/csc.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>

SteeringOrchestrator::SteeringOrchestrator(SteeringControllerConfig &config)
    : m_config(config), m_impulsefilter(m_config.mechanics, m_config.physics),
      m_csc(m_config.regulations), m_pwm() {}
void SteeringOrchestrator::tick(NavigationSensors::NavigationSnapshot snapshot, uint16_t cscInternalTarget,
                                uint32_t loopTimestamp) {

  //TODO 
  //Evtl zu methodenaufrufen umstruktieren. Vorher Abhängigkeiten untersuchen 
  //Gerade unklar ob Target über Ticks hinaus gehalten werden soll
  m_csc.currentHDG(snapshot.compass_hdg_dg.value);
  m_csc.setInternalTarget(cscInternalTarget);
  auto intentOpt = m_csc.tick(loopTimestamp);

  if (intentOpt) {
    SteeringIntent intent = *intentOpt;
    auto cmd = m_impulsefilter.apply(intent, snapshot);
    m_pwm.command(cmd);
  }
};
