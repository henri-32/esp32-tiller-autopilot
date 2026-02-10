#pragma once

#include "diagnostics/diagnostics.h"
#include "sensors/navigationSensors.h"
#include "types/globalTypes.h"
#include <array>
#include <cstdint>

class PWMController;

class SteeringController {
public:
  explicit SteeringController(PWMController &pwm, NavigationSensors &navsens,
                              SteeringController_Config &config, Diagnostics& diagnostics);

  void tick(unsigned long loopTimestamp);
  void setCurrentCompassCourse(uint16_t currentCourse);

  class SourceEvaluator {
  public:
    explicit SourceEvaluator(NavigationSensors &navsens,
                            SteeringController_Config &config, Diagnostics& diagnostics);

    void tick(unsigned long loopTimestamp);

  private:
    NavigationSensors &m_navigationSensors;
    SteeringController_Config &m_steeringController_Config;
    Diagnostics& m_diagnostics;
  };

  class CoreSteeringController {
  public:
    explicit CoreSteeringController(PWMController &pwm,
                                    NavigationSensors &navsens,
                                    SteeringController_Config &config);

    void tick(unsigned long loopTimestamp);
    void setCurrentCompassCourse(uint16_t currentCourse);

  private:
    PWMController &m_pwm;
    NavigationSensors &m_navigationSensors;
    const SteeringController_Config &m_SteeringController_Config;

    static constexpr uint8_t OBSERVATION_BUFFER_SIZE =
        SteeringController_Config::observationBufferSize;

    uint16_t m_internalTargetCourse = 0;
    uint16_t m_currentCourse = 0;

    int16_t calculateHeadingError(uint16_t current, uint16_t target);
    void updateObservationBuffers(unsigned long loopTimestamp);
    bool steeringCorrectionIsRequired();
    SteeringDirection calculateSteeringDirectionFromObservation();
    void resetObservations();
    bool observationAllowed(unsigned long loopTimestamp) const;
    bool actionAllowed(unsigned long loopTimestamp) const;

    std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_leftErrors;
    std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_rightErrors;
    std::array<int16_t, OBSERVATION_BUFFER_SIZE> m_mergedError;

    int16_t m_medianOfMergedErrors = 0;

    unsigned long m_lastImpulse = 0;
    unsigned long m_lastObservation = 0;

    uint8_t m_observationCount = 0;
  };

private:
  CoreSteeringController m_csc;
   Diagnostics& m_diagnostics; 
};
