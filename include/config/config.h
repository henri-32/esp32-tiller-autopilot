#pragma once
#include <cstdint>
#include "driver/gpio.h"

struct SteeringMechanicsConfig
{
    uint8_t dutyCycleDefault = 40;
    uint32_t steeringMinImpulse_ms = 100;
    uint32_t steeringMaxImpulse_ms = 3000;
};

struct SteeringRegulationConfig
{
    // Regulation thresholds and timing gates used by CSC.
    uint8_t steeringTolerance_deg = 1;
    uint8_t counterNearTargetWindow_deg = 2;
    int8_t smoothedMeanApplicationWindow_deg = 20;
    uint32_t calculationWindowSmoothedMean = 4;
    float omegaDeadband = 0.2;
    static constexpr uint8_t observationBufferSize = 70;
    // Runtime-active capacity used by CSC. Defaults to compile-time maximum.
    uint8_t activeObservationBufferSize = observationBufferSize;
    uint8_t minimumSampleSize = 10;
    uint8_t omegaRobust = 3; // Describes Samplesize (errors) for Omega calculation
    float omegaThresholdForCounter = 0.02;
    uint8_t counterOmegaMinSampleSize = 3;
    uint32_t minimumTimeBtwObs_ms = 1000;
    uint32_t pauseForValidObsAfterImpulse_ms = 3000;
    uint32_t steeringCooldown_ms = 2000;
    uint32_t counterTimerGuard_ms = 2000;
    uint32_t counterCooldown_ms = 7000;
};

struct SteeringSourceHandlingConfig
{
    // Policy thresholds for selecting and validating sources.
    float minGpsSpeedForUse = 1.0;
    uint32_t minTimeBetweenCorrection_ms = 20000;
    static constexpr uint8_t observationBufferSize = 70;
};
struct SteeringPhysicsConfig
{
    // Damping model for impulse shaping based on STW.
    float STWDampingRegulator = 0.4f;
    const float STWDampingMinFactor = 0.35f;
    const float STWDampingMaxFactor = 1.0f;
};

struct DisplayConfig 
{
	// DisplaySettings 
	uint32_t refreshRate_ms = 1000;
	uint16_t displayWidth = 1080; 
	uint16_t displayHeight = 1440; 
};

struct SteeringControllerConfig
{
    // Aggregate config owned by the SystemController.
    SteeringMechanicsConfig mechanics;
    SteeringRegulationConfig regulations;
    SteeringPhysicsConfig physics;
    SteeringSourceHandlingConfig source;
	DisplayConfig display;

};

