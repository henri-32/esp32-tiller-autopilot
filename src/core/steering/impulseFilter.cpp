#include "core/steering/impulseFilter.h"
#include "types/globalTypes.h"
#include <algorithm>
#include <cstdint>


/*Für mein zukünftiges Ich. Die Syntax mit return .x ist ziemlich cool, weil ich
damit keine lokale Variable des Rückgabewerts zwischenspeichern muss, sondern
direkt auf die Argumente des Rückgabewerts schreibe */

ImpulseFilter::ImpulseFilter(const SteeringMechanicsConfig &mechanicsConfig,
                             const SteeringPhysicsConfig &physicsConfig)
    : m_mechanics(mechanicsConfig), m_physics(physicsConfig) {};

PWMIntent
ImpulseFilter::apply(const SteeringIntent &intent,
                     const NavigationSensors::NavigationSnapshot &snapshot) {

  if (!snapshot.stw_kts.valid) {
    return {
        .dir = intent.dir,
        .filteredAbstractImpulse_0_100 = intent.abstractImpulse_0_100,
        .dutyCycle = m_mechanics.dutyCycleDefault,
    };
  };

  const float v = snapshot.stw_kts.value;

  float damping =
      1.0f / (1.0f + m_physics.STWDampingRegulator * v * v); // Physik Bruder

  damping = std::clamp(damping, m_physics.STWDampingMinFactor,
                       m_physics.STWDampingMaxFactor);

  float abstract = intent.abstractImpulse_0_100;
  float effective = abstract * damping;
  float normalized = effective / 100;

  return {
      .dir = intent.dir,
      .filteredAbstractImpulse_0_100 = static_cast<uint16_t>(normalized*100),
      .dutyCycle = m_mechanics.dutyCycleDefault,

  };
}
