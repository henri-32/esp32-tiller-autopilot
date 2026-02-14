#include "core/steering/sourceHandling.h"

SourceHandler::SourceHandler(const ControlPanel &panel,
                             NavigationSensors &navsens,
                             SteeringSourceHandlingConfig &config,
                             Diagnostics &diagnostics,
                             CoreSteeringController &csc)
    : m_panel(panel), m_navigationSensors(navsens),
      m_diagnostics(diagnostics),
      m_csc(csc), m_sourcePolicy(config, diagnostics),
      m_targetInterpreter(m_gpsModel, m_windModel),
      m_sourceExecutor(navsens, csc) {}

void SourceHandler::tick(NavigationSource requestedSource,
                         uint32_t loopTimestamp,
                         NavigationSensors::NavigationSnapshot snapshot,
                         uint16_t generalTarget) {
  // Apply source policy and fallback rules.
  NavigationSource effective =
      m_sourcePolicy.evaluate(requestedSource, snapshot, loopTimestamp);

  // Filter the target based on the effective source.
  uint16_t filteredTarget =
      m_targetInterpreter.applySourceFilters(generalTarget, effective);

  // Propagate the effective source and filtered target to the steering stack.
  m_sourceExecutor.execute(effective, filteredTarget);
};

//__________SOURCE_POLICY_______________________________________________
SourcePolicyEngine::SourcePolicyEngine(SteeringSourceHandlingConfig &config,
                                       Diagnostics &diagnostics)
    : m_config(config), m_diagnostics(diagnostics) {}

NavigationSource
SourcePolicyEngine::evaluate(NavigationSource requested,
                             NavigationSensors::NavigationSnapshot snapshot,
                             uint32_t loopTimestamp) {
  NavigationSource effective = requested;

  switch (requested) {
  case NavigationSource::Gps:
    if (!snapshot.gps_cog_dg.valid || !snapshot.gps_sog_kts.valid) {
      effective = NavigationSource::Compass;
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                         FunctionalCapability::GPS, loopTimestamp);

    } else if (snapshot.gps_sog_kts.value <
               m_config.minGpsSpeedForUse) {
      effective = NavigationSource::Compass;
      m_diagnostics.emit(DiagnosticEvent::Degraded, FunctionalCapability::GPS,
                         loopTimestamp);

    } else {
      effective = NavigationSource::Gps;
    };
    break;
  case NavigationSource::Wind:
    if (!snapshot.wind_angle_dg.valid) {
      effective = NavigationSource::Compass;
      m_diagnostics.emit(DiagnosticEvent::LostWithFallback,
                         FunctionalCapability::WIND, loopTimestamp);
    } else {
      effective = NavigationSource::Wind;
    };
    break;

  case NavigationSource::Compass:
    if (!snapshot.compass_hdg_dg.valid) {
      m_diagnostics.emit(DiagnosticEvent::CRITICAL_ERROR,
                         FunctionalCapability::COMPASS, loopTimestamp);
    }
  };

  return effective;
};

//_____________________SOURCE_INTERPRETER________________________
SourceTargetInterpreter::SourceTargetInterpreter(GPSModel &gpsModel,
                                                 WindModel &windModel)
    : m_gpsModel(gpsModel), m_windModel(windModel) {}

uint16_t
SourceTargetInterpreter::applySourceFilters(uint16_t generalTarget,
                                            NavigationSource effective) {
  uint16_t filteredTarget = generalTarget;

  switch (effective) {
  case NavigationSource::Gps:
    filteredTarget = m_gpsModel.applyFilter(generalTarget);
    break;
  case NavigationSource::Wind:
    filteredTarget = m_windModel.applyFilter(generalTarget);
    break;
  case NavigationSource::Compass:
    filteredTarget = generalTarget;
  }

  return filteredTarget;
}

//____________________________EXECUTOR_______________________________

SourceExecutor::SourceExecutor(NavigationSensors &navsens,
                               CoreSteeringController &csc)
    : m_navigationSensors(navsens), m_csc(csc) {}

void SourceExecutor::execute(NavigationSource effective,
                             uint16_t filteredTarget) {

  m_navigationSensors.setLeadSource(effective);
  m_csc.setInternalTarget(filteredTarget);
}
