#include "core/steering/csc/steeringGuard.h"

SteeringGuard::SteeringGuard(const SteeringRegulationConfig &config)
    : m_config(config), m_deadband(config){};

bool SteeringGuard::observationBlocked(uint32_t lastUpdate,
                                       uint32_t lastIntent,
                                       uint32_t loopTimestamp) {
  // Mindestabstand zwischen zwei Beobachtungen.
  if (loopTimestamp - lastUpdate < m_config.minimumTimeBtwObs_ms) {
    return true;
  };

  // Nach einem Intent kurz nicht beobachten, bis sich das System beruhigt hat.
  if (loopTimestamp - lastIntent < m_config.pauseForValidObsAfterImpulse_ms) {
    return true;
  };

  return false;
};

bool SteeringGuard::intentBlocked(uint32_t lastIntent, int16_t median,
                                  uint8_t sampleSize, float omega,
                                  uint32_t loopTimestamp) {
  // Guard-Ebene ist hier Takt-/Stabilitaetsschutz der Regelung.
  // Ein zusaetzlicher, rein hardwarebezogener Schutz kann spaeter im
  // PWM-Controller ergaenzt werden.

  // Cooldown zwischen zwei Intents.
  if (loopTimestamp - lastIntent < m_config.steeringCooldown_ms) {
    return true;
  };

  // Vor einer Aktion muss genug Evidenz im Buffer sein.
  if (sampleSize < m_config.minimumSampleSize) {
    return true;
  };

  // Der Median ist die Action/NoAction-Entscheidungsbasis.
  // Einzelwerte bleiben zwar im Buffer, aber der Intent wird nur bei
  // signifikantem Median freigegeben.
  if (!m_deadband.errorSignificant(median)) {
    return true;
  };

  /* Omega-Guard:
  Haben median und omega das gleiche Vorzeichen und liegt |omega| ausserhalb
  des Deadbands, bewegt sich der Fehler bereits in die richtige Richtung.
  Dann wird kein zusaetzlicher Intent ausgeloest.

  TODO (fehlerabhaengige Omega-Schwelle):
  Der Guard ist aktuell binaer und kann bei grossem Fehler zu langen
  No-Intent-Phasen fuehren. Sinnvoll waere eine Mindest-|omega|, die von der
  Fehlergroesse abhaengt, damit bei grossem Fehler frueher wieder Intents
  erlaubt werden. 
  Um das sinnvoll zu implementieren müssten reale omega werte aus den Praxistests
  herausgefunden werden. */
  if (median < 0 && omega < 0 - m_config.omegaDeadband) {
    return true;
  };
  if (median > 0 && omega > 0 + m_config.omegaDeadband) {
    return true;
  };

  return false;
};
