#include "core/steering/sourceEvaluator.h"

SourceEvaluator::SourceEvaluator(
    NavigationSensors &navsens, SteeringController_Config &config,
    Diagnostics &diagnostics)
    : m_navigationSensors(navsens), m_steeringController_Config(config),
      m_diagnostics(diagnostics) {};

void SourceEvaluator::tick(
    NavigationSensors::NavigationSnapshot snapshot, uint32_t loopTimestamp) {
  switch (m_navigationSensors.getLeadSource()) {
  case NavigationSource::Gps:
    if (!snapshot.gps_cog_dg.valid) {
      m_navigationSensors.setLeadSource(NavigationSource::Compass);
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                         FunctionalCapability::GPS, loopTimestamp);

      // Behandlung geschwindigkeit zu gering fÃ¼r Regelung
      // Alles noch in einzelne Funktion auslagern
    };
    break;

  case NavigationSource::Compass:
    if (!snapshot.compass_hdg_dg.valid) {
      m_navigationSensors.setLeadSource(NavigationSource::Gps);
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                         FunctionalCapability::COMPASS, loopTimestamp);
      // SystemController muss noch auswerten, wie lange lost, damit dann
      // critical Error. Das Fallback kann nur Ã¼bergangsweise erfolgen.
    };
    break;

  case NavigationSource::Wind:
    if (!snapshot.wind_angle_dg.valid) {
      m_navigationSensors.setLeadSource(NavigationSource::Compass);
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                         FunctionalCapability::WIND, loopTimestamp);
    }
    break;
  };
};