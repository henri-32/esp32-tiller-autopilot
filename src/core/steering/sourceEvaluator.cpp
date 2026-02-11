#include "core/steering/sourceEvaluator.h"
#include "core/steering/csc.h"
#include "diagnostics/diagnostic_types.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <cstdint>


SourceEvaluator::SourceEvaluator(NavigationSensors &navsens,
                                 SteeringController_Config &config,
                                 Diagnostics &diagnostics)
    : m_navigationSensors(navsens), m_steeringController_Config(config),
      m_diagnostics(diagnostics) {};

void SourceEvaluator::tick(NavigationSensors::NavigationSnapshot snapshot,
                           CoreSteeringController &csc,
                           uint32_t loopTimestamp) {
  switch (m_navigationSensors.getLeadSource()) {
  case NavigationSource::Gps:
    evaluateGPS(snapshot, loopTimestamp);
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

void SourceEvaluator::evaluateGPS(
    NavigationSensors::NavigationSnapshot snapshot, uint32_t loopTimestamp) {
  if (!snapshot.gps_cog_dg.valid || !snapshot.gps_sog_kts.valid) {
    m_navigationSensors.setLeadSource(NavigationSource::Compass);
    m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                       FunctionalCapability::GPS, loopTimestamp);
  };

  if (snapshot.gps_sog_kts.value <
      m_steeringController_Config.source.minimumGPS_SpeedForGPS_Use) {
    m_navigationSensors.setLeadSource(NavigationSource::Compass);
    m_diagnostics.emit(DiagnosticEvent::Degraded, FunctionalCapability::GPS,
                       loopTimestamp);
  };

  // Logik der csc sollkursanpassung
};

void SourceEvaluator::evaluateCompass(
    NavigationSensors::NavigationSnapshot snapshot, CoreSteeringController &csc,
    uint32_t loopTimestamp) {
  if (!snapshot.compass_hdg_dg.valid) {
    // entweder direkt Critical Error, oder hier übergangsregelung auf cog bauen
  };

  csc.setInternalTarget(
      300 /* TODO Control Panel target modellieren und hier einfügen*/);
};
