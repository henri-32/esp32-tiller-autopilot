#include "core/steering/steeringOrchestrator.h"
#include "core/steering/csc/csc.h"
#include "drivers/navigationSensors.h"
#include "types/steeringTypes.h"
#include <cstdint>

SteeringOrchestrator::SteeringOrchestrator(
    const SteeringControllerConfig &config)
    : m_impulsefilter(config.mechanics, config.physics),
      m_csc(config.regulations), m_pwm(config.mechanics) {}
void SteeringOrchestrator::tick(
    const NavigationSensors::NavigationSnapshot &snapshot,
                                uint16_t cscInternalTarget,
                                uint32_t loopTimestamp) {

  // TODO
  // Evtl zu methodenaufrufen umstruktieren. Vorher Abhängigkeiten untersuchen
  // Gerade unklar ob Target über Ticks hinaus gehalten werden soll
  // TODO(Architektur):
  // CSC soll "nur Kurse" sehen. Der direkte Zugriff auf
  // snapshot.compass_hdg_dg ist aktuell bewusst Compass-only, koppelt den
  // Orchestrator aber an Snapshot-Details. Falls spaeter ein GuidanceOutput
  // existiert, sollte currentHDG nur noch daraus kommen.
  m_csc.currentHDG(snapshot.compass_hdg_dg.value);
  m_csc.setInternalTarget(cscInternalTarget);
  auto intentOpt = m_csc.tick(loopTimestamp);

  if (intentOpt) {
    auto cmd = m_impulsefilter.apply(*intentOpt, snapshot);
    m_pwm.command(cmd);
  }
};
