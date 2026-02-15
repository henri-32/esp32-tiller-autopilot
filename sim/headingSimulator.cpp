#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>

#include "core/config.h"
#include "core/steering/csc/csc.h"
#include "headingSimulator.h"
#include "types/globalTypes.h"

namespace{
constexpr const char *kOutputPath = "sim/sim_output/drift_sim.csv";
}

SimulationConfig makeDefaultSimulationConfig() {
  SimulationConfig config{};

  // --- Time ---
  config.dt_sec = 0.5f;
  config.sim_seconds = 180;

  // --- Boot dynamics ---
  config.intentStepToOmega = 0.01f;
  config.initialHeading_deg = 10.0f;
  config.environmentTorque_deg_s2 = 0.0f;

  // --- CSC ---
  config.regulation.steeringTolerance_deg = 5;
  config.regulation.minimumSampleSize = 5;
  config.regulation.minimumTimeBtwObs_ms = 500;
  config.regulation.pauseForValidObsAfterImpulse_ms = 3000;
  config.regulation.steeringCooldown_ms = 2000;
  config.target_deg = 0;

  return config;
}

BootModel::BootModel(float startHeading_deg, float intentStepToOmega)
    : m_heading_deg(normalizeHeading(startHeading_deg)),
      m_angularVelocity_deg_s(0.0f),
      m_intentStepToOmega(intentStepToOmega) {}

void BootModel::applyIntent(float signedImpulse_0_100) {
  m_angularVelocity_deg_s += m_intentStepToOmega * signedImpulse_0_100;
}

void BootModel::update(float dt, float externalTorque_deg_s2) {
  m_angularVelocity_deg_s += externalTorque_deg_s2 * dt;
  m_heading_deg = normalizeHeading(m_heading_deg + (m_angularVelocity_deg_s * dt));
}

float BootModel::heading() const { return m_heading_deg; }

float BootModel::angularVelocity() const { return m_angularVelocity_deg_s; }

float BootModel::normalizeHeading(float heading_deg) {
  while (heading_deg < 0.0f) {
    heading_deg += 360.0f;
  }
  while (heading_deg >= 360.0f) {
    heading_deg -= 360.0f;
  }
  return heading_deg;
}

EnvironmentModel::EnvironmentModel(float constantTorque_deg_s2)
    : m_constantTorque_deg_s2(constantTorque_deg_s2) {}

float EnvironmentModel::externalTorque(float /*time_s*/) const {
  return m_constantTorque_deg_s2;
}

SimulationEngine::SimulationEngine(CoreSteeringController &csc,
                                   const SimulationConfig &config)
    : m_boot(config.initialHeading_deg, config.intentStepToOmega),
      m_environment(config.environmentTorque_deg_s2),
      m_csc(csc) {}

void SimulationEngine::tick(float dt, uint32_t loopTimestamp) {
  const uint16_t heading_u16 =
      static_cast<uint16_t>(m_boot.heading()) % static_cast<uint16_t>(360);
  m_csc.currentHDG(heading_u16);

  m_lastIntent = m_csc.tick(loopTimestamp);

  const float time_s = static_cast<float>(loopTimestamp) * 0.001f;
  const float envTorque = m_environment.externalTorque(time_s);

  if (m_lastIntent.has_value()) {
    const float magnitude =
        static_cast<float>(m_lastIntent->abstractImpulse_0_100);
    const float signedImpulse =
        (m_lastIntent->dir == SteeringDirection::Left) ? -magnitude : magnitude;
    m_boot.applyIntent(signedImpulse);
  }
  m_boot.update(dt, envTorque);
}

float SimulationEngine::heading() const { return m_boot.heading(); }

float SimulationEngine::angularVelocity() const { return m_boot.angularVelocity(); }

std::optional<SteeringIntent> SimulationEngine::lastIntent() const {
  return m_lastIntent;
}

int main() {
  SimulationConfig config = makeDefaultSimulationConfig();

  const uint32_t dt_ms = static_cast<uint32_t>(config.dt_sec * 1000.0f);
  const uint32_t steps =
      static_cast<uint32_t>(static_cast<float>(config.sim_seconds) / config.dt_sec);

  CoreSteeringController csc(config.regulation);
  csc.setInternalTarget(config.target_deg);

  SimulationEngine engine(csc, config);

  // ----------------------------
  // CSV
  // ----------------------------
  std::ofstream file(kOutputPath);
  if (!file.is_open()) {
    std::cerr << "Failed to open simulation output: " << kOutputPath << "\n";
    return 1;
  }

  file << "time_ms,"
       << "heading_deg,"
       << "angular_velocity_deg_s,"
       << "target_deg,"
       << "error_deg,"
       << "median_deg,"
       << "sample_size,"
       << "deadband_active,"
       << "obs_blocked,"
       << "intent_blocked,"
       << "intent,"
       << "dir\n";

  file << std::fixed << std::setprecision(3);

  uint32_t time_ms = 0;

  for (uint32_t i = 0; i < steps; ++i) {
    engine.tick(config.dt_sec, time_ms);
    const auto intent = engine.lastIntent();
    const float heading = engine.heading();
    const float angularVelocity = engine.angularVelocity();

    const auto &dbg = csc.getDebug();

    file << time_ms << "," << heading << "," << angularVelocity << ","
         << config.target_deg << "," << dbg.error << "," << dbg.median << ","
         << static_cast<int>(dbg.sampleSize) << ","
         << dbg.deadbandActive << "," << dbg.observationBlocked << ","
         << dbg.intentBlocked << "," << (intent.has_value() ? 1 : 0) << ",";

    if (intent) {
      file << (intent->dir == SteeringDirection::Left ? "Left" : "Right");
    }

    file << "\n";

    time_ms += dt_ms;
  }

  file.close();
  return 0;
}
