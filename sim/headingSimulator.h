#pragma once

#include <cstdint>
#include <optional>

#include "core/steering/csc/csc.h"

struct SimulationConfig {
  // --- Time ---
  float dt_sec;
  uint32_t sim_seconds;

  // --- Boot dynamics ---
  float intentStepToOmega;
  float initialHeading_deg;
  float environmentTorque_deg_s2;

  // --- CSC ---
  SteeringRegulationConfig regulation;
  uint16_t target_deg;
};

SimulationConfig makeDefaultSimulationConfig();

class BootModel {
public:
  BootModel(float startHeading_deg, float intentStepToOmega);

  // Intent effect is applied once per intent event.
  void applyIntent(float signedImpulse_0_100);

  // Minimal step model:
  // omega += externalTorque * dt
  // heading += omega * dt
  void update(float dt, float externalTorque_deg_s2);

  float heading() const;
  float angularVelocity() const;

private:
  static float normalizeHeading(float heading_deg);
  float m_heading_deg = 0.0f;
  float m_angularVelocity_deg_s = 0.0f;
  float m_intentStepToOmega = 0.0f;
};

class EnvironmentModel {
public:
  explicit EnvironmentModel(float constantTorque_deg_s2 = 0.0f);

  // Minimal disturbance model; constant torque for now.
  float externalTorque(float time_s) const;

private:
  float m_constantTorque_deg_s2 = 0.0f;
};

class SimulationEngine {
public:
  SimulationEngine(CoreSteeringController &csc, const SimulationConfig &config);

  // Orchestrates one simulation step:
  // 1) heading -> CSC
  // 2) CSC tick
  // 3) intent impulse -> BootModel update
  void tick(float dt, uint32_t loopTimestamp);

  float heading() const;
  float angularVelocity() const;
  std::optional<SteeringIntent> lastIntent() const;

private:
  BootModel m_boot;
  EnvironmentModel m_environment;
  CoreSteeringController &m_csc;
  std::optional<SteeringIntent> m_lastIntent;
};
