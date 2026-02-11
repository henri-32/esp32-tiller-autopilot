#include "core/steering/impulseFilter.h"
#include "types/globalTypes.h"
#include <algorithm>
#include <cstdint>

/*Für mein zukünftiges Ich. Die Syntax mit return .x ist ziemlich cool, weil ich
damit keine lokale Variable des Rückgabewerts zwischenspeichern muss, sondern
direkt auf die Argumente des Rückgabewerts schreibe */

ImpulseFilter::ImpulseFilter(SteeringController_Config &config)
    : m_config(config) {};

PWMCommand
ImpulseFilter::apply(SteeringIntent intent,
                     NavigationSensors::NavigationSnapshot snapshot) {

  if (!snapshot.stw_kts.valid) {
    return {
        .dir = intent.dir,
        .pulse_ms = m_config.mechanics.SteeringMaxImpulse_ms,
        .dutyCycle = m_config.mechanics.dutyCycleDefault,
    };
  };

  const float v = snapshot.stw_kts.value;

  float damping =
      1.0f / (1.0f + m_config.physics.stwDampingRegulator * v * v); // Physik Bruder

  damping = std::clamp(damping, m_config.physics.stwDampingMinFactor,
                       m_config.physics.stwDampingMaxFactor);

  float abstract =
      intent.abstractImpulse_0_100; // Ist bei Aufruf von CSC immer 100
  float effective = abstract * damping;
  float normalized = effective / 100;

  // pulse is mapped into [min, max].
  // Zero pulse is not a valid hardware state.
  // "No action" is modeled via std::nullopt at CSC level.

  uint32_t pulse_ms = m_config.mechanics.SteeringMinImpulse_ms +
                      normalized * (m_config.mechanics.SteeringMaxImpulse_ms -
                                    m_config.mechanics.SteeringMinImpulse_ms);

  return {
      .dir = intent.dir,
      .pulse_ms = pulse_ms,
      .dutyCycle = m_config.mechanics.dutyCycleDefault,

  };
}
