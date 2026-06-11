#pragma once

#include <cstdint>
#include <optional>

#include "core/steering/csc/csc.h"

struct BoatPhysicsConfig {
  float intentStepToTiller;
  float tillerMaxAbs;
  float tillerToYawAccel_deg_s2;
  float initialHeading_deg;
  float environmentTorque_deg_s2;
  float yawInertia;
  float yawDampingLinear;
  float yawDampingQuadratic;
  float trueWindSpeed_mps;
  float trueWindDirection_deg;
  float windHullYawAccelPerMps2;
};

struct SimulationConfig {
  // --- Time ---
  float dt_sec;
  uint32_t sim_seconds;

  // --- Boot dynamics ---
  BoatPhysicsConfig boat;

  // --- CSC ---
  SteeringRegulationConfig regulation;
  uint16_t target_deg;
};

SimulationConfig makeDefaultSimulationConfig();

class BootModel {
public:
  explicit BootModel(const BoatPhysicsConfig &config);

  // Intent changes tiller position and stays latched (no auto recentering).
  void applyIntent(float signedImpulse_0_100);

  // Simplified yaw dynamics:
  // omega += (env + wind + tiller - damping) * dt
  // heading += omega * dt
  void update(float dt);

  float heading() const;
  float angularVelocity() const;
  float tillerPosition() const;
  float tillerYawAccel() const;
  float windYawAccel() const;
  float dampingYawAccel() const;

private:
  static float normalizeAngleSignedDeg(float angle_deg);
  static float normalizeHeading(float heading_deg);
  static float degToRad(float deg);
  float computeWindYawAccel() const;

  float m_heading_deg = 0.0f;
  float m_angularVelocity_deg_s = 0.0f;
  float m_tillerPosition = 0.0f;
  float m_lastTillerYawAccel_deg_s2 = 0.0f;
  float m_lastWindYawAccel_deg_s2 = 0.0f;
  float m_lastDampingYawAccel_deg_s2 = 0.0f;

  float m_intentStepToTiller = 0.0f;
  float m_tillerMaxAbs = 1.0f;
  float m_tillerToYawAccel_deg_s2 = 0.0f;
  float m_environmentTorque_deg_s2 = 0.0f;
  float m_yawInertia = 1.0f;
  float m_yawDampingLinear = 0.0f;
  float m_yawDampingQuadratic = 0.0f;
  float m_trueWindSpeed_mps = 0.0f;
  float m_trueWindDirection_deg = 0.0f;
  float m_windHullYawAccelPerMps2 = 0.0f;
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
  float tillerPosition() const;
  float tillerYawAccel() const;
  float windYawAccel() const;
  float dampingYawAccel() const;
  std::optional<SteeringIntent> lastIntent() const;

private:
  BootModel m_boot;
  CoreSteeringController &m_csc;
  std::optional<SteeringIntent> m_lastIntent;
};
