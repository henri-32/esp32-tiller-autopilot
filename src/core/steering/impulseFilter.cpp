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
        .pulse_ms = m_config.SteeringImpulseDefault_ms,
        .dutyCycle = m_config.dutyCycleDefault,
    };
  };

  const float v = snapshot.stw_kts.value;

  float damping =
      1.0f / (1.0f + m_config.stwDampingRegulator * v * v); // Physik Bruder

  damping = std::clamp(damping, m_config.stwDampingMinFactor,
                       m_config.stwDampingMaxFactor);

  float basepulse = m_config.SteeringImpulseDefault_ms *
                    (intent.abstractImpulse_0_100 / 100.0f);

  return {
      .dir = intent.dir,
      .pulse_ms = static_cast<uint32_t>(basepulse * damping),
      .dutyCycle = m_config.dutyCycleDefault,

  };
}
