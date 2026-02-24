#include "core/steering/impulseFilter.h"
#include "types/globalTypes.h"
#include <algorithm>
#include <cstdint>

/*Diese Klasse bekommt die abstrakte gewünschte Impulsstärke vom CSC 
Die wird in dieser Klasse abhängig vom Speed through water intepretiert und 
als pwm command an den pwm Regler übergeben 
TODO 
Semantisch müsste dann eigentlich noch der abstrakte Impuls weiter gefiltert werden 
und erst der PWM Controller rechnet das in Impulszeiten um*/


/*Für mein zukünftiges Ich. Die Syntax mit return .x ist ziemlich cool, weil ich
damit keine lokale Variable des Rückgabewerts zwischenspeichern muss, sondern
direkt auf die Argumente des Rückgabewerts schreibe */

ImpulseFilter::ImpulseFilter(SteeringMechanicsConfig& mechanicsConfig, SteeringPhysicsConfig& physicsConfig)
    : m_mechanics(mechanicsConfig), m_physics(physicsConfig) {};

PWMCommand
ImpulseFilter::apply(SteeringIntent intent,
                     NavigationSensors::NavigationSnapshot snapshot) {

  if (!snapshot.stw_kts.valid) {
    return {
        .dir = intent.dir,
        .pulse_ms = intent.abstractImpulse_0_100,
        .dutyCycle = m_mechanics.dutyCycleDefault,
    };
  };

  const float v = snapshot.stw_kts.value;

  float damping = 1.0f / (1.0f + m_physics.STWDampingRegulator * v *
                                     v); // Physik Bruder

  damping = std::clamp(damping, m_physics.STWDampingMinFactor,
                       m_physics.STWDampingMaxFactor);

  float abstract =
      intent.abstractImpulse_0_100;
  float effective = abstract * damping;
  float normalized = effective / 100;

  // pulse is mapped into [min, max].
  // Zero pulse is not a valid hardware state.
  // "No action" is modeled via std::nullopt at CSC level.

  uint32_t pulse_ms = m_mechanics.steeringMinImpulse_ms +
                      normalized * (m_mechanics.steeringMaxImpulse_ms -
                                    m_mechanics.steeringMinImpulse_ms);

  return {
      .dir = intent.dir,
      .pulse_ms = pulse_ms,
      .dutyCycle = m_mechanics.dutyCycleDefault,

  };
}
