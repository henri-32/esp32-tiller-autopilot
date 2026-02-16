#include <cstdint>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <regex>
#include <sstream>
#include <string>

#include "core/config.h"
#include "core/steering/csc/csc.h"
#include "headingSimulator.h"
#include "types/globalTypes.h"

namespace {
constexpr const char *kOutputPath = "sim/sim_output/drift_sim.csv";
constexpr const char *kConfigSnapshotJsonPath =
    "sim/sim_output/drift_sim_config.json";
constexpr const char *kNextRunConfigPath = "sim/next_run_config.json";

bool readFileToString(const char *path, std::string &content) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Failed to open simulation config input: " << path << "\n";
    return false;
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();
  content = buffer.str();
  return true;
}

bool extractNumber(const std::string &json, const char *key, double &value) {
  const std::regex pattern("\"" + std::string(key) + "\"\\s*:\\s*([-+]?[0-9]*\\.?[0-9]+)");
  std::smatch match;
  if (!std::regex_search(json, match, pattern)) {
    std::cerr << "Missing numeric key in next-run config: " << key << "\n";
    return false;
  }

  try {
    value = std::stod(match[1].str());
  } catch (const std::exception &) {
    std::cerr << "Invalid numeric value for key in next-run config: " << key
              << "\n";
    return false;
  }

  return true;
}

bool toFloat(double input, const char *key, float &output) {
  if (!std::isfinite(input)) {
    std::cerr << "Invalid non-finite float for key in next-run config: " << key
              << "\n";
    return false;
  }
  output = static_cast<float>(input);
  return true;
}

bool toUint32(double input, const char *key, uint32_t &output) {
  if (!std::isfinite(input) || input < 0.0 ||
      input > static_cast<double>(std::numeric_limits<uint32_t>::max()) ||
      std::floor(input) != input) {
    std::cerr << "Invalid uint32 value for key in next-run config: " << key
              << "\n";
    return false;
  }

  output = static_cast<uint32_t>(input);
  return true;
}

bool toUint16(double input, const char *key, uint16_t &output) {
  if (!std::isfinite(input) || input < 0.0 ||
      input > static_cast<double>(std::numeric_limits<uint16_t>::max()) ||
      std::floor(input) != input) {
    std::cerr << "Invalid uint16 value for key in next-run config: " << key
              << "\n";
    return false;
  }

  output = static_cast<uint16_t>(input);
  return true;
}

bool toUint8(double input, const char *key, uint8_t &output) {
  if (!std::isfinite(input) || input < 0.0 ||
      input > static_cast<double>(std::numeric_limits<uint8_t>::max()) ||
      std::floor(input) != input) {
    std::cerr << "Invalid uint8 value for key in next-run config: " << key
              << "\n";
    return false;
  }

  output = static_cast<uint8_t>(input);
  return true;
}

bool loadSimulationConfigFromJson(const char *path, SimulationConfig &config) {
  std::string json;
  if (!readFileToString(path, json)) {
    return false;
  }

  double dt_sec = 0.0;
  double sim_seconds = 0.0;
  double intentStepToOmega = 0.0;
  double initialHeading_deg = 0.0;
  double environmentTorque_deg_s2 = 0.0;
  double steeringTolerance_deg = 0.0;
  double minimumSampleSize = 0.0;
  double minimumTimeBtwObs_ms = 0.0;
  double pauseForValidObsAfterImpulse_ms = 0.0;
  double steeringCooldown_ms = 0.0;
  double calculationWindowSmoothedMean = 0.0;
  double target_deg = 0.0;

  if (!extractNumber(json, "dt_sec", dt_sec) ||
      !extractNumber(json, "sim_seconds", sim_seconds) ||
      !extractNumber(json, "intentStepToOmega", intentStepToOmega) ||
      !extractNumber(json, "initialHeading_deg", initialHeading_deg) ||
      !extractNumber(json, "environmentTorque_deg_s2", environmentTorque_deg_s2) ||
      !extractNumber(json, "steeringTolerance_deg", steeringTolerance_deg) ||
      !extractNumber(json, "minimumSampleSize", minimumSampleSize) ||
      !extractNumber(json, "minimumTimeBtwObs_ms", minimumTimeBtwObs_ms) ||
      !extractNumber(json, "pauseForValidObsAfterImpulse_ms",
                     pauseForValidObsAfterImpulse_ms) ||
      !extractNumber(json, "steeringCooldown_ms", steeringCooldown_ms) ||
      !extractNumber(json, "calculationWindowSmoothedMean",
                     calculationWindowSmoothedMean) ||
      !extractNumber(json, "target_deg", target_deg)) {
    return false;
  }

  if (!toFloat(dt_sec, "dt_sec", config.dt_sec) ||
      !toUint32(sim_seconds, "sim_seconds", config.sim_seconds) ||
      !toFloat(intentStepToOmega, "intentStepToOmega", config.intentStepToOmega) ||
      !toFloat(initialHeading_deg, "initialHeading_deg",
               config.initialHeading_deg) ||
      !toFloat(environmentTorque_deg_s2, "environmentTorque_deg_s2",
               config.environmentTorque_deg_s2) ||
      !toUint8(steeringTolerance_deg, "steeringTolerance_deg",
               config.regulation.steeringTolerance_deg) ||
      !toUint8(minimumSampleSize, "minimumSampleSize",
               config.regulation.minimumSampleSize) ||
      !toUint32(minimumTimeBtwObs_ms, "minimumTimeBtwObs_ms",
                config.regulation.minimumTimeBtwObs_ms) ||
      !toUint32(pauseForValidObsAfterImpulse_ms,
                "pauseForValidObsAfterImpulse_ms",
                config.regulation.pauseForValidObsAfterImpulse_ms) ||
      !toUint32(steeringCooldown_ms, "steeringCooldown_ms",
                config.regulation.steeringCooldown_ms) ||
      !toUint32(calculationWindowSmoothedMean,
                "calculationWindowSmoothedMean",
                config.regulation.calculationWindowSmoothedMean) ||
      !toUint16(target_deg, "target_deg", config.target_deg)) {
    return false;
  }

  std::cout << "Loaded simulation config from: " << path << "\n";
  return true;
}

void writeConfigSnapshotJson(const SimulationConfig &config) {
  std::ofstream file(kConfigSnapshotJsonPath);
  if (!file.is_open()) {
    std::cerr << "Failed to open simulation config json snapshot: "
              << kConfigSnapshotJsonPath << "\n";
    return;
  }

  file << std::fixed << std::setprecision(3);
  file << "{\n";
  file << "  \"time\": {\n";
  file << "    \"dt_sec\": " << config.dt_sec << ",\n";
  file << "    \"sim_seconds\": " << config.sim_seconds << "\n";
  file << "  },\n";
  file << "  \"boat\": {\n";
  file << "    \"intentStepToOmega\": " << config.intentStepToOmega << ",\n";
  file << "    \"initialHeading_deg\": " << config.initialHeading_deg << ",\n";
  file << "    \"environmentTorque_deg_s2\": " << config.environmentTorque_deg_s2
       << "\n";
  file << "  },\n";
  file << "  \"csc\": {\n";
  file << "    \"steeringTolerance_deg\": "
       << static_cast<int>(config.regulation.steeringTolerance_deg) << ",\n";
  file << "    \"minimumSampleSize\": "
       << static_cast<int>(config.regulation.minimumSampleSize) << ",\n";
  file << "    \"minimumTimeBtwObs_ms\": "
       << config.regulation.minimumTimeBtwObs_ms << ",\n";
  file << "    \"pauseForValidObsAfterImpulse_ms\": "
       << config.regulation.pauseForValidObsAfterImpulse_ms << ",\n";
  file << "    \"steeringCooldown_ms\": "
       << config.regulation.steeringCooldown_ms << ",\n";
  file << "    \"calculationWindowSmoothedMean\": "
       << config.regulation.calculationWindowSmoothedMean << ",\n";
  file << "    \"target_deg\": " << config.target_deg << "\n";
  file << "  }\n";
  file << "}\n";
}
}

SimulationConfig makeDefaultSimulationConfig() {
  SimulationConfig config{};

  // --- Time ---
  config.dt_sec = 0.1f;
  config.sim_seconds = 180;

  // --- Boot dynamics ---
  config.intentStepToOmega =
      0.005f; // Weil abstract intent noch 100 ist wirkt dieser parameter *100
              // also 0.1 = 1deg/sec
  config.initialHeading_deg = 15.0f;
  config.environmentTorque_deg_s2 = 0.00f;

  // --- CSC ---
  config.regulation.steeringTolerance_deg = 7;
  config.regulation.minimumSampleSize = 5;
  config.regulation.minimumTimeBtwObs_ms = 100;
  config.regulation.pauseForValidObsAfterImpulse_ms = 2000;
  config.regulation.steeringCooldown_ms = 500;
  config.regulation.calculationWindowSmoothedMean = 4;
  config.target_deg = 0;

  return config;
}

BootModel::BootModel(float startHeading_deg, float intentStepToOmega)
    : m_heading_deg(normalizeHeading(startHeading_deg)),
      m_angularVelocity_deg_s(0.0f), m_intentStepToOmega(intentStepToOmega) {}

void BootModel::applyIntent(float signedImpulse_0_100) {
  m_angularVelocity_deg_s += m_intentStepToOmega * signedImpulse_0_100;
}

void BootModel::update(float dt, float externalTorque_deg_s2) {
  m_angularVelocity_deg_s += externalTorque_deg_s2 * dt;
  m_heading_deg =
      normalizeHeading(m_heading_deg + (m_angularVelocity_deg_s * dt));
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
      m_environment(config.environmentTorque_deg_s2), m_csc(csc) {}

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

float SimulationEngine::angularVelocity() const {
  return m_boot.angularVelocity();
}

std::optional<SteeringIntent> SimulationEngine::lastIntent() const {
  return m_lastIntent;
}

int main() {
  SimulationConfig config = makeDefaultSimulationConfig();
  if (!loadSimulationConfigFromJson(kNextRunConfigPath, config)) {
    return 1;
  }
  writeConfigSnapshotJson(config);

  const uint32_t dt_ms = static_cast<uint32_t>(config.dt_sec * 1000.0f);
  const uint32_t steps = static_cast<uint32_t>(
      static_cast<float>(config.sim_seconds) / config.dt_sec);

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
         << static_cast<int>(dbg.sampleSize) << "," << dbg.deadbandActive << ","
         << dbg.observationBlocked << "," << dbg.intentBlocked << ","
         << (intent.has_value() ? 1 : 0) << ",";

    if (intent) {
      file << (intent->dir == SteeringDirection::Left ? "Left" : "Right");
    }

    file << "\n";

    time_ms += dt_ms;
  }

  file.close();
  return 0;
}
