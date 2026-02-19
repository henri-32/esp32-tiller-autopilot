#include "core/steering/csc/csc.h"

#include "core/steering/csc/deadband.h"
#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/steeringGuard.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

CoreSteeringController::CoreSteeringController(SteeringRegulationConfig &config)
    : m_observationBuffer(config), m_deadband(config), m_steeringGuard(config),
      m_config(config) {}

std::optional<SteeringIntent>
CoreSteeringController::tick(uint32_t loopTimestamp) {
  // Debug flags are per-tick state, not latched state.
  m_debug.observationBlocked = false;
  m_debug.intentBlocked = false;
  m_debug.deadbandActive = false;

  /*CSC macht zwei Sachen
  - Beobachten
  - Intent zurückgeben

  Wenn er wegen Guards nicht beobachten darf early return*/
  if (m_steeringGuard.observationBlocked(loopTimestamp, m_lastObsUpdate,
                                         m_lastIntent)) {
    m_debug.observationBlocked = true;
    return std::nullopt;
  }

  int16_t error = m_errorCalculator.getCurrentError(m_currentCourse,
                                                    m_internalTargetCourse);
  m_debug.error = error;

  m_observationBuffer.update(error, loopTimestamp);
  m_lastObsUpdate = loopTimestamp;

  // Bewusst kein Struct, damit gezielte Zugriffe live möglich bleiben und nicht
  // an snapshots gebunden wird
  const int16_t median = m_observationBuffer.getMedian();
  const uint8_t sampleSize = m_observationBuffer.getSampleSize();
  const float omega = m_observationBuffer.getOmega(loopTimestamp);

  m_debug.median = median;
  m_debug.sampleSize = sampleSize;

  /*Counter Intent steuert kurz vor Erreichen des Targets gegen.
  Siehe Doku iterations/3. counterImpulse
  Absichtlich außerhalb der regulären Intent-Guards, weil eigene Guards
  Vor den regulären Intents, damit er nicht von deren Guards abgehalten wird*/
  if (counterIntentNecessary(loopTimestamp, median, sampleSize, omega)) {
    auto counter = counterIntent(omega);
    m_lastIntent = loopTimestamp;
    m_lastCounterIntent = loopTimestamp;
    m_lastIntentValue = counter;
    m_observationBuffer.reset();
    return counter;
  }

  /*steering Guards.*/
  if (m_steeringGuard.intentBlocked(loopTimestamp, m_lastIntent, sampleSize,
                                    median, omega)) {
    m_debug.intentBlocked = true;

    // Beim Fehler von 0 wird der Buffer zurückgesetzt, um schnelleres Reagieren
    // bei Richtungswechseln zu ermöglichen, dadurch dass der Median um null
    // weniger robust wird. Die steeringTolerance ermöglicht Oszillationen um 0
    // als no action zu behandeln, während große Fehler schneller signifikant
    // werden
    if (std::abs(error) <= 1) {
      m_observationBuffer.reset();
    }

    return std::nullopt;
  } else {

    auto intent = calculateIntentFromObs();
    m_lastIntent = loopTimestamp;

    /* Wenn Handlung ausgelöst wird, wurde auf Evidenz reagiert und
      diese wird bewusst verworfen*/
    m_observationBuffer.reset();

    /*Für Counter Intent wichtig*/
    if (intent.has_value()) {
      m_lastIntentValue = intent;
    }

    return intent;
  }
}

void CoreSteeringController::currentHDG(uint16_t current) {
  m_currentCourse = current;
}

void CoreSteeringController::setInternalTarget(uint16_t target) {
  m_internalTargetCourse = target;
};

uint16_t CoreSteeringController::getInternalTarget() const {
  return m_internalTargetCourse;
};

std::optional<SteeringIntent> CoreSteeringController::calculateIntentFromObs() {

  /* Der Median wird als robuster Mittelwert für die Entscheidung
  Action/NoAction und Richtungsentscheidung genutzt. Ein leicht geglätteter
  aktueller mean wird als Fehlergröße für die Berechnung der abstrakten
  Impulsstärke genutzt. */

  SteeringIntent intent; // return value

  intent.dir = determineDirection(m_observationBuffer.getMedian());

  /* Semantisch klargestellt. CSC setzt grundsätzlich bei jeder Entscheidung
  vollen abstrakten Impuls.
  Der kann danach nach unten gedämpft, aber nicht nach
  oben eskaliert werden.*/
  intent.abstractImpulse_0_100 = 100;
  int16_t currentError = m_observationBuffer.getSmoothedCurrentError();

  if (currentError < -m_config.smoothedMeanApplicationWindow_deg ||
      currentError > m_config.smoothedMeanApplicationWindow_deg) {
    return intent;
  };

  /*Bis hier hin war Richtung noch wichtig, um mean korrekt bestimmen zu
  können Ab hier nur noch für abstractImpulse Größe ohne Vorzeichenrelevanz
  verwendet*/
  currentError = std::abs(currentError);

  /*Wenn die obere (positive) Grenze des Windows 100% abstract Impulse bedeutet,
  wird so in linearem Verhältnis auf die Fehlergröße reagiert*/
  float errorPercantage = static_cast<float>(currentError) /
                          m_config.smoothedMeanApplicationWindow_deg;
  intent.abstractImpulse_0_100 *= errorPercantage;

  return intent;
};

SteeringDirection
CoreSteeringController::determineDirection(int16_t median) const {
  if (median < 0) {
    return SteeringDirection::Left;
  } else if (median > 0) {
    return SteeringDirection::Right;
  };
  return SteeringDirection::Left;
}

bool CoreSteeringController::counterIntentNecessary(uint32_t loopTimestamp,
                                                    int16_t median,
                                                    uint8_t sampleSize,
                                                    float omega) {
  // nullopt kann aufgrund fehlender Richtung nicht Basis sein
  if (!m_lastIntentValue.has_value()) {
    return false;
  }

  // FlipFlop Guard (nach FlipFlops in Sims) Bewusst auf reguläre Intents
  // bezogen
  if (loopTimestamp - m_lastIntent < m_config.counterTimerGuard_ms &&
      m_lastIntentValue->dir != determineDirection(median)) {
    return false;
  }
  // Cooldown auf letzte counters bezogen
  if (loopTimestamp - m_lastCounterIntent < m_config.counterCooldown_ms) {
    return false;
  }

  // Qualität von Omega
  if (sampleSize < m_config.counterOmegaMinSampleSize ||
      std::abs(omega) < m_config.omegaThresholdForCounter) {
    return false;
  }

  const int16_t absError = std::abs(m_debug.error);
  const bool nearTarget = absError <= m_config.counterNearTargetWindow_deg;

  if (median < 0 && omega < 0 && nearTarget) {
    return true;
  } else if (median > 0 && omega > 0 && nearTarget) {
    return true;
  } else
    return false;
};

SteeringIntent CoreSteeringController::counterIntent(float omega) {
  SteeringIntent intent;
  if (m_lastIntentValue->dir == SteeringDirection::Left) {
    intent.dir = SteeringDirection::Right;
  } else if (m_lastIntentValue->dir == SteeringDirection::Right) {
    intent.dir = SteeringDirection::Left;
  }

  /*TODO Der abstract Impulse sollte auf die ersten realen Testergebnisse
  (Einfluss von Intent auf Omega) gemappt werden, damit das Target möglichst
  genau abgefangen werden kann.
  Hier ist gerade SIM Logik drin !!!!!!
  Ich fange 60% von Omega ab um zu gucken, ob ich eingeregelt bekomme!!!!!*/
  const float omegaAbs = std::abs(omega);
  const float mappedImpulse = omegaAbs * 100.0f * 0.62f;
  const float clampedImpulse = std::clamp(mappedImpulse, 0.0f, 100.0f);
  intent.abstractImpulse_0_100 = static_cast<uint8_t>(clampedImpulse);

  return intent;
};

const CSCDebugSnapshot &CoreSteeringController::getDebug() const {
  return m_debug;
}
