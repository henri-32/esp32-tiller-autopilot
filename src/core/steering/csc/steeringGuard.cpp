#include "core/steering/csc/steeringGuard.h"

SteeringGuard::SteeringGuard(SteeringRegulationConfig &config)
    : m_config(config), m_deadband(config) {};

bool SteeringGuard::observationBlocked(uint32_t loopTimestamp,
                                       uint32_t lastUpdate,
                                       uint32_t lastIntent) {
  // Minimum time between observations.
  if (loopTimestamp - lastUpdate < m_config.minimumTimeBtwObs_ms) {
    return true;
  };

  // Pause observations right after an impulse to wait for stabilization.
  if (loopTimestamp - lastIntent < m_config.pauseForValidObsAfterImpulse_ms) {
    return true;
  };

  return false;
};

bool SteeringGuard::intentBlocked(uint32_t loopTimestamp, uint32_t lastIntent,
                                  uint8_t sampleSize, int16_t median, float omega) {
  /*Semantisch eigentlich Hardwareschutz kommt vllt in den PWM später*/
  // Cooldown between actuator commands.
  if (loopTimestamp - lastIntent < m_config.steeringCooldown_ms) {
    return true;
  };

  // Require a minimum number of samples before commanding.
  if (sampleSize < m_config.minimumSampleSize) {
    return true;
  };

  // Der Median ist Action/NoAction Entscheidungsgrundlage
  // Daher ist er ein Intent guard. Errors werden unabhängig von der Toleranz im
  // Buffer gespeichert
  if (!m_deadband.errorSignificant(median)) {
    return true;
  };

  /* Wenn die Winkelgeschwindigkeit das gleiche Vorzeichen wie der Median hat 
  hat sie eine andere semantische Richtung (Fehler ist, was zum Target fehlt, nicht was vom Target weg ist)
  In dem Fall bewegt sich das System in die Richtige Richtung und es soll nicht zusätzlich gesteuert werden

  TODO (Omega-Guard fehlerabhängig machen):
  Der aktuelle Guard ist binär und kann bei großem Anfangsfehler zu langen
  No-Intent-Phasen führen, sobald omega "formal richtig" ist (gleiches
  Vorzeichen wie median). In Simulationen entstehen dadurch teils >10s bis
  ~20s ohne neuen Intent, obwohl |error| noch deutlich groß ist.

  Ziel:
  - No-Action nur dann, wenn omega für die aktuelle Fehlergröße "ausreichend"
    ist, nicht allein wegen Vorzeichen.

  Möglicher Ansatz:
  - Fehler in Größenklassen einteilen (z.B. |error| > 20, >10, >5, <=5).
  - Pro Klasse eine minimale erforderliche |omega|-Schwelle definieren
    (bei großem Fehler höhere Mindest-omega, nahe Ziel niedrigere).
  - Guard nur aktivieren, wenn
      sign(median) == sign(omega) UND |omega| >= omegaMinForError(|error|).
    Wenn omega zwar richtiges Vorzeichen hat, aber zu klein ist:
      -> Guard NICHT aktivieren (Intent erlauben).

  Erwartete Wirkung:
  - Große Fehler werden schneller eingefangen (weniger "zu frühes Zurücklehnen").
  - Nahe am Ziel bleibt der dämpfende Effekt des Guards erhalten.
  - Parametrierung wird transparenter, weil Guard-Verhalten explizit an
    Fehlerregime gekoppelt ist. */
  if (median < 0 && omega < 0 - m_config.omegaDeadband){
    return true;
  };
  if(median > 0 && omega > 0 + m_config.omegaDeadband){
    return true;
  };

  return false;
};
