#include "core/steering/sourceEvaluator.h"
#include "core/steering/csc/csc.h"
#include "diagnostics/diagnostic_types.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include "ui/controlPanel.h"
#include <cstdint>

SourceEvaluator::SourceEvaluator(const ControlPanel &panel,
                                 NavigationSensors &navsens,
                                 SteeringController_Config &config,
                                 Diagnostics &diagnostics)
    : m_panel(panel), m_navigationSensors(navsens),
      m_steeringController_Config(config), m_diagnostics(diagnostics) {};

void SourceEvaluator::tick(NavigationSensors::NavigationSnapshot snapshot,
                           CoreSteeringController &csc,
                           uint32_t loopTimestamp) {
  switch (m_navigationSensors.getLeadSource()) {
  case NavigationSource::Gps:
    evaluateGPS(snapshot, csc, loopTimestamp);
    break;

  case NavigationSource::Compass:
    evaluateCompass(snapshot, csc, loopTimestamp);
    break;

  case NavigationSource::Wind:
    if (!snapshot.wind_angle_dg.valid) {
      m_navigationSensors.setLeadSource(NavigationSource::Compass);
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                         FunctionalCapability::WIND, loopTimestamp);
      return;
    }
    evaluateWind(snapshot, csc, loopTimestamp);
    break;

  default:
    m_navigationSensors.setLeadSource(NavigationSource::Compass);
  };
};

void SourceEvaluator::evaluateGPS(
    NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController &csc,
    uint32_t loopTimestamp) {
  if (!snapshot.gps_cog_dg.valid || !snapshot.gps_sog_kts.valid) {
    m_navigationSensors.setLeadSource(NavigationSource::Compass);
    m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                       FunctionalCapability::GPS, loopTimestamp);
    return;
  };

  if (snapshot.gps_sog_kts.value <
      m_steeringController_Config.source.minimumGPS_SpeedForGPS_Use) {
    m_navigationSensors.setLeadSource(NavigationSource::Compass);
    m_diagnostics.emit(DiagnosticEvent::Degraded, FunctionalCapability::GPS,
                       loopTimestamp);
    return;
  };

  uint16_t targetFromGPS = 0;
  // TODO Logik der GPS Target Implementierung

  csc.setInternalTarget(targetFromGPS);
};

void SourceEvaluator::evaluateCompass(
    NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController &csc,
    uint32_t loopTimestamp) {
  if (!snapshot.compass_hdg_dg.valid) {
    // entweder direkt Critical Error, oder hier übergangsregelung auf cog bauen
  };

  csc.setInternalTarget(m_panel.readIntent().generalTarget);
};

void SourceEvaluator::evaluateWind(
    NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController &csc,
    uint32_t loopTimestamp) {

  if (!snapshot.wind_angle_dg.valid) {
    m_navigationSensors.setLeadSource(NavigationSource::Gps);
    m_diagnostics.emit(DiagnosticEvent::Degraded, FunctionalCapability::WIND,
                       loopTimestamp);
    return;
  }

  if (loopTimestamp - m_lastObservation <
      m_steeringController_Config.regulations.minimumTimeBtwObs_ms) {
    return;
  };
 
  // Hier ist noch ein kernfehler drin, weil generalTarget nicht genutzt wird.
  uint16_t generalTarget =
      m_panel.readIntent()
          .generalTarget; // Wird bei Windmodus zu einem Windwinkel
  uint16_t currentHDG = snapshot.compass_hdg_dg.value;
  uint16_t rawWindAngle = snapshot.wind_angle_dg.value;
  uint16_t diff = currentHDG - rawWindAngle;

  diff += 360;
  diff += 180;
  diff %= 360;
  diff -= 180;
  int16_t normalizedDiff = diff; 

  m_observationBuffer[m_observationCounter] = normalizedDiff;
  m_lastObservation = loopTimestamp;
  m_observationCounter++;

  uint16_t sum = 0;
  for (int i = 0; i < OBSERVATION_BUFFER_SIZE; i++) {
    sum += m_observationBuffer[i];
  };

  int16_t mean = sum / OBSERVATION_BUFFER_SIZE;

  if (loopTimestamp - m_lastCorrection <
      m_steeringController_Config.source.minimumTimeBeweenCorrection_ms) {
    return;
  }

  if (mean < 0) {
    csc.setInternalTarget(csc.getInternalTarget() + mean);
  }

  else if (mean > 0) {
    csc.setInternalTarget(csc.getInternalTarget() - mean);
  }
};


