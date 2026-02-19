#include <cstdint>
#include <cmath>
#include <filesystem>
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
constexpr const char *kOutputDirectoryPath = "sim/sim_output";
constexpr const char *kNextRunConfigPath = "sim/next_run_config.json";

bool ensureOutputDirectoryExists() {
  std::error_code ec;
  std::filesystem::create_directories(kOutputDirectoryPath, ec);
  if (ec) {
    std::cerr << "Failed to create simulation output directory: "
              << kOutputDirectoryPath << " (" << ec.message() << ")\n";
    return false;
  }
  return true;
}

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

bool toPositiveInt8(double input, const char *key, int8_t &output) {
  if (!std::isfinite(input) || input <= 0.0 ||
      input > static_cast<double>(std::numeric_limits<int8_t>::max()) ||
      std::floor(input) != input) {
    std::cerr << "Invalid positive int8 value for key in next-run config: "
              << key << "\n";
    return false;
  }

  output = static_cast<int8_t>(input);
  return true;
}

bool loadSimulationConfigFromJson(const char *path, SimulationConfig &config) {
  std::string json;
  if (!readFileToString(path, json)) {
    return false;
  }

  double dt_sec = 0.0;
  double sim_seconds = 0.0;
  double intentStepToTiller = 0.0;
  double tillerMaxAbs = 0.0;
  double tillerToYawAccel_deg_s2 = 0.0;
  double initialHeading_deg = 0.0;
  double environmentTorque_deg_s2 = 0.0;
  double yawInertia = 0.0;
  double yawDampingLinear = 0.0;
  double yawDampingQuadratic = 0.0;
  double trueWindSpeed_mps = 0.0;
  double trueWindDirection_deg = 0.0;
  double windHullYawAccelPerMps2 = 0.0;
  double steeringTolerance_deg = 0.0;
  double counterNearTargetWindow_deg = 0.0;
  double counterTimerGuard_ms = 0.0;
  double counterCooldown_ms = 0.0;
  double counterOmegaMinSampleSize = 0.0;
  double omegaThresholdForCounter = 0.0;
  double minimumSampleSize = 0.0;
  double minimumTimeBtwObs_ms = 0.0;
  double pauseForValidObsAfterImpulse_ms = 0.0;
  double steeringCooldown_ms = 0.0;
  double calculationWindowSmoothedMean = 0.0;
  double smoothedMeanApplicationWindow_deg = 20.0;
  double observationBufferSize = 0.0;
  double omegaRobust = 0.0;
  double omegaDeadband = 0.0;
  double target_deg = 0.0;


  if (!extractNumber(json, "dt_sec", dt_sec) ||
      !extractNumber(json, "sim_seconds", sim_seconds) ||
      !extractNumber(json, "intentStepToTiller", intentStepToTiller) ||
      !extractNumber(json, "tillerMaxAbs", tillerMaxAbs) ||
      !extractNumber(json, "tillerToYawAccel_deg_s2", tillerToYawAccel_deg_s2) ||
      !extractNumber(json, "initialHeading_deg", initialHeading_deg) ||
      !extractNumber(json, "environmentTorque_deg_s2",
                     environmentTorque_deg_s2) ||
      !extractNumber(json, "yawInertia", yawInertia) ||
      !extractNumber(json, "yawDampingLinear", yawDampingLinear) ||
      !extractNumber(json, "yawDampingQuadratic", yawDampingQuadratic) ||
      !extractNumber(json, "trueWindSpeed_mps", trueWindSpeed_mps) ||
      !extractNumber(json, "trueWindDirection_deg", trueWindDirection_deg) ||
      !extractNumber(json, "windHullYawAccelPerMps2",
                     windHullYawAccelPerMps2) ||
      !extractNumber(json, "steeringTolerance_deg", steeringTolerance_deg) ||
      !extractNumber(json, "counterNearTargetWindow_deg",
                     counterNearTargetWindow_deg) ||
      !extractNumber(json, "counterTimerGuard_ms", counterTimerGuard_ms) ||
      !extractNumber(json, "counterCooldown_ms", counterCooldown_ms) ||
      !extractNumber(json, "counterOmegaMinSampleSize",
                     counterOmegaMinSampleSize) ||
      !extractNumber(json, "omegaThresholdForCounter",
                     omegaThresholdForCounter) ||
      !extractNumber(json, "minimumSampleSize", minimumSampleSize) ||
      !extractNumber(json, "minimumTimeBtwObs_ms", minimumTimeBtwObs_ms) ||
      !extractNumber(json, "pauseForValidObsAfterImpulse_ms",
                     pauseForValidObsAfterImpulse_ms) ||
      !extractNumber(json, "steeringCooldown_ms", steeringCooldown_ms) ||
      !extractNumber(json, "calculationWindowSmoothedMean",
                     calculationWindowSmoothedMean) ||
      !extractNumber(json, "smoothedMeanApplicationWindow_deg",
                     smoothedMeanApplicationWindow_deg) ||
      !extractNumber(json, "observationBufferSize", observationBufferSize) ||
      !extractNumber(json, "omegaRobust", omegaRobust) ||
      !extractNumber(json, "omegaDeadband", omegaDeadband) ||
      !extractNumber(json, "target_deg", target_deg)) {
    return false;
  }

  if (!toFloat(dt_sec, "dt_sec", config.dt_sec) ||
      !toUint32(sim_seconds, "sim_seconds", config.sim_seconds) ||
      !toFloat(intentStepToTiller, "intentStepToTiller",
               config.boat.intentStepToTiller) ||
      !toFloat(tillerMaxAbs, "tillerMaxAbs", config.boat.tillerMaxAbs) ||
      !toFloat(tillerToYawAccel_deg_s2, "tillerToYawAccel_deg_s2",
               config.boat.tillerToYawAccel_deg_s2) ||
      !toFloat(initialHeading_deg, "initialHeading_deg",
               config.boat.initialHeading_deg) ||
      !toFloat(environmentTorque_deg_s2, "environmentTorque_deg_s2",
               config.boat.environmentTorque_deg_s2) ||
      !toFloat(yawInertia, "yawInertia", config.boat.yawInertia) ||
      !toFloat(yawDampingLinear, "yawDampingLinear",
               config.boat.yawDampingLinear) ||
      !toFloat(yawDampingQuadratic, "yawDampingQuadratic",
               config.boat.yawDampingQuadratic) ||
      !toFloat(trueWindSpeed_mps, "trueWindSpeed_mps",
               config.boat.trueWindSpeed_mps) ||
      !toFloat(trueWindDirection_deg, "trueWindDirection_deg",
               config.boat.trueWindDirection_deg) ||
      !toFloat(windHullYawAccelPerMps2, "windHullYawAccelPerMps2",
               config.boat.windHullYawAccelPerMps2) ||
      !toUint8(steeringTolerance_deg, "steeringTolerance_deg",
               config.regulation.steeringTolerance_deg) ||
      !toUint8(counterNearTargetWindow_deg, "counterNearTargetWindow_deg",
               config.regulation.counterNearTargetWindow_deg) ||
      !toUint32(counterTimerGuard_ms, "counterTimerGuard_ms",
                config.regulation.counterTimerGuard_ms) ||
      !toUint32(counterCooldown_ms, "counterCooldown_ms",
                config.regulation.counterCooldown_ms) ||
      !toUint8(counterOmegaMinSampleSize, "counterOmegaMinSampleSize",
               config.regulation.counterOmegaMinSampleSize) ||
      !toFloat(omegaThresholdForCounter, "omegaThresholdForCounter",
               config.regulation.omegaThresholdForCounter) ||
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
      !toPositiveInt8(smoothedMeanApplicationWindow_deg,
                      "smoothedMeanApplicationWindow_deg",
                      config.regulation.smoothedMeanApplicationWindow_deg) ||
      !toUint8(observationBufferSize, "observationBufferSize",
               config.regulation.activeObservationBufferSize) ||
      !toUint8(omegaRobust, "omegaRobust", config.regulation.omegaRobust) ||
      !toFloat(omegaDeadband, "omegaDeadband", config.regulation.omegaDeadband) ||
      !toUint16(target_deg, "target_deg", config.target_deg)) {
    return false;
  }

  if (config.dt_sec <= 0.0f) {
    std::cerr
        << "Invalid value for key in next-run config: dt_sec must be > 0\n";
    return false;
  }

  if (config.sim_seconds == 0U) {
    std::cerr
        << "Invalid value for key in next-run config: sim_seconds must be > 0\n";
    return false;
  }

  if (config.regulation.counterNearTargetWindow_deg == 0) {
    std::cerr << "Invalid value for key in next-run config: "
              << "counterNearTargetWindow_deg must be >= 1\n";
    return false;
  }

  if (config.regulation.counterOmegaMinSampleSize == 0) {
    std::cerr << "Invalid value for key in next-run config: "
              << "counterOmegaMinSampleSize must be >= 1\n";
    return false;
  }

  if (config.regulation.activeObservationBufferSize == 0) {
    std::cerr << "Invalid value for key in next-run config: "
              << "observationBufferSize must be >= 1\n";
    return false;
  }

  if (config.regulation.activeObservationBufferSize >
      SteeringRegulationConfig::observationBufferSize) {
    std::cerr << "Invalid value for key in next-run config: "
              << "observationBufferSize exceeds compile-time max "
              << static_cast<int>(SteeringRegulationConfig::observationBufferSize)
              << "\n";
    return false;
  }

  if (config.regulation.minimumSampleSize >
      config.regulation.activeObservationBufferSize) {
    std::cerr << "Invalid config combination in next-run config: "
              << "minimumSampleSize must be <= observationBufferSize\n";
    return false;
  }

  if (config.regulation.omegaDeadband < 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "omegaDeadband must be >= 0\n";
    return false;
  }

  if (config.regulation.omegaThresholdForCounter < 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "omegaThresholdForCounter must be >= 0\n";
    return false;
  }

  if (config.boat.yawInertia <= 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "yawInertia must be > 0\n";
    return false;
  }

  if (config.boat.intentStepToTiller < 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "intentStepToTiller must be >= 0\n";
    return false;
  }

  if (config.boat.tillerMaxAbs <= 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "tillerMaxAbs must be > 0\n";
    return false;
  }

  if (config.boat.tillerToYawAccel_deg_s2 < 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "tillerToYawAccel_deg_s2 must be >= 0\n";
    return false;
  }

  if (config.boat.yawDampingLinear < 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "yawDampingLinear must be >= 0\n";
    return false;
  }

  if (config.boat.yawDampingQuadratic < 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "yawDampingQuadratic must be >= 0\n";
    return false;
  }

  if (config.boat.trueWindSpeed_mps < 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "trueWindSpeed_mps must be >= 0\n";
    return false;
  }

  if (config.boat.windHullYawAccelPerMps2 < 0.0f) {
    std::cerr << "Invalid value for key in next-run config: "
              << "windHullYawAccelPerMps2 must be >= 0\n";
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
  file << "    \"intentStepToTiller\": " << config.boat.intentStepToTiller
       << ",\n";
  file << "    \"tillerMaxAbs\": " << config.boat.tillerMaxAbs << ",\n";
  file << "    \"tillerToYawAccel_deg_s2\": "
       << config.boat.tillerToYawAccel_deg_s2
       << ",\n";
  file << "    \"initialHeading_deg\": " << config.boat.initialHeading_deg
       << ",\n";
  file << "    \"environmentTorque_deg_s2\": "
       << config.boat.environmentTorque_deg_s2 << ",\n";
  file << "    \"yawInertia\": " << config.boat.yawInertia << ",\n";
  file << "    \"yawDampingLinear\": " << config.boat.yawDampingLinear
       << ",\n";
  file << "    \"yawDampingQuadratic\": " << config.boat.yawDampingQuadratic
       << ",\n";
  file << "    \"trueWindSpeed_mps\": " << config.boat.trueWindSpeed_mps
       << ",\n";
  file << "    \"trueWindDirection_deg\": " << config.boat.trueWindDirection_deg
       << ",\n";
  file << "    \"windHullYawAccelPerMps2\": "
       << config.boat.windHullYawAccelPerMps2 << "\n";
  file << "  },\n";
  file << "  \"csc\": {\n";
  file << "    \"steeringTolerance_deg\": "
       << static_cast<int>(config.regulation.steeringTolerance_deg) << ",\n";
  file << "    \"counterNearTargetWindow_deg\": "
       << static_cast<int>(config.regulation.counterNearTargetWindow_deg)
       << ",\n";
  file << "    \"counterTimerGuard_ms\": "
       << config.regulation.counterTimerGuard_ms << ",\n";
  file << "    \"counterCooldown_ms\": "
       << config.regulation.counterCooldown_ms << ",\n";
  file << "    \"counterOmegaMinSampleSize\": "
       << static_cast<int>(config.regulation.counterOmegaMinSampleSize)
       << ",\n";
  file << "    \"omegaThresholdForCounter\": "
       << config.regulation.omegaThresholdForCounter << ",\n";
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
  file << "    \"smoothedMeanApplicationWindow_deg\": "
       << static_cast<int>(config.regulation.smoothedMeanApplicationWindow_deg)
       << ",\n";
  file << "    \"observationBufferSize\": "
       << static_cast<int>(config.regulation.activeObservationBufferSize)
       << ",\n";
  file << "    \"omegaRobust\": "
       << static_cast<int>(config.regulation.omegaRobust)
       << ",\n";
  file << "    \"omegaDeadband\": "
       << config.regulation.omegaDeadband
       << ",\n";
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

  // --- Boat dynamics ---
  config.boat.intentStepToTiller = 0.005f;
  config.boat.tillerMaxAbs = 1.0f;
  config.boat.tillerToYawAccel_deg_s2 = 1.5f;
  config.boat.initialHeading_deg = 15.0f;
  config.boat.environmentTorque_deg_s2 = 0.0f;
  config.boat.yawInertia = 1.0f;
  config.boat.yawDampingLinear = 0.30f;
  config.boat.yawDampingQuadratic = 0.02f;
  config.boat.trueWindSpeed_mps = 6.0f;
  config.boat.trueWindDirection_deg = 70.0f;
  config.boat.windHullYawAccelPerMps2 = 0.03f;

  // --- CSC ---
  config.regulation.steeringTolerance_deg = 7;
  config.regulation.counterNearTargetWindow_deg = 2;
  config.regulation.counterTimerGuard_ms = 2000;
  config.regulation.counterCooldown_ms = 7000;
  config.regulation.counterOmegaMinSampleSize = 3;
  config.regulation.omegaThresholdForCounter = 0.02f;
  config.regulation.minimumSampleSize = 5;
  config.regulation.minimumTimeBtwObs_ms = 100;
  config.regulation.pauseForValidObsAfterImpulse_ms = 2000;
  config.regulation.steeringCooldown_ms = 500;
  config.regulation.calculationWindowSmoothedMean = 4;
  config.regulation.smoothedMeanApplicationWindow_deg = 20;
  config.regulation.activeObservationBufferSize =
      SteeringRegulationConfig::observationBufferSize;
  config.regulation.omegaRobust = 3;
  config.regulation.omegaDeadband = 0.2f;
  config.target_deg = 0;

  return config;
}

BootModel::BootModel(const BoatPhysicsConfig &config)
    : m_heading_deg(normalizeHeading(config.initialHeading_deg)),
      m_angularVelocity_deg_s(0.0f),
      m_intentStepToTiller(config.intentStepToTiller),
      m_tillerMaxAbs(config.tillerMaxAbs),
      m_tillerToYawAccel_deg_s2(config.tillerToYawAccel_deg_s2),
      m_environmentTorque_deg_s2(config.environmentTorque_deg_s2),
      m_yawInertia(config.yawInertia),
      m_yawDampingLinear(config.yawDampingLinear),
      m_yawDampingQuadratic(config.yawDampingQuadratic),
      m_trueWindSpeed_mps(config.trueWindSpeed_mps),
      m_trueWindDirection_deg(config.trueWindDirection_deg),
      m_windHullYawAccelPerMps2(config.windHullYawAccelPerMps2) {}

void BootModel::applyIntent(float signedImpulse_0_100) {
  m_tillerPosition += m_intentStepToTiller * signedImpulse_0_100;
  if (m_tillerPosition > m_tillerMaxAbs) {
    m_tillerPosition = m_tillerMaxAbs;
  } else if (m_tillerPosition < -m_tillerMaxAbs) {
    m_tillerPosition = -m_tillerMaxAbs;
  }
}

void BootModel::update(float dt) {
  const float windYawAccel_deg_s2 = computeWindYawAccel();
  const float tillerNormalized = (m_tillerMaxAbs > 0.0f)
                                     ? (m_tillerPosition / m_tillerMaxAbs)
                                     : 0.0f;
  const float tillerYawAccel_deg_s2 =
      tillerNormalized * m_tillerToYawAccel_deg_s2;

  const float dampingMoment =
      (m_yawDampingLinear * m_angularVelocity_deg_s) +
      (m_yawDampingQuadratic * std::fabs(m_angularVelocity_deg_s) *
       m_angularVelocity_deg_s);
  const float dampingYawAccel_deg_s2 = dampingMoment / m_yawInertia;

  m_lastTillerYawAccel_deg_s2 = tillerYawAccel_deg_s2;
  m_lastWindYawAccel_deg_s2 = windYawAccel_deg_s2;
  m_lastDampingYawAccel_deg_s2 = -dampingYawAccel_deg_s2;

  const float totalYawAccel_deg_s2 =
      m_environmentTorque_deg_s2 + windYawAccel_deg_s2 +
      tillerYawAccel_deg_s2 - dampingYawAccel_deg_s2;

  m_angularVelocity_deg_s += totalYawAccel_deg_s2 * dt;
  m_heading_deg =
      normalizeHeading(m_heading_deg + (m_angularVelocity_deg_s * dt));
}

float BootModel::heading() const { return m_heading_deg; }

float BootModel::angularVelocity() const { return m_angularVelocity_deg_s; }

float BootModel::tillerPosition() const { return m_tillerPosition; }

float BootModel::tillerYawAccel() const { return m_lastTillerYawAccel_deg_s2; }

float BootModel::windYawAccel() const { return m_lastWindYawAccel_deg_s2; }

float BootModel::dampingYawAccel() const { return m_lastDampingYawAccel_deg_s2; }

float BootModel::normalizeHeading(float heading_deg) {
  while (heading_deg < 0.0f) {
    heading_deg += 360.0f;
  }
  while (heading_deg >= 360.0f) {
    heading_deg -= 360.0f;
  }
  return heading_deg;
}

float BootModel::normalizeAngleSignedDeg(float angle_deg) {
  while (angle_deg > 180.0f) {
    angle_deg -= 360.0f;
  }
  while (angle_deg <= -180.0f) {
    angle_deg += 360.0f;
  }
  return angle_deg;
}

float BootModel::degToRad(float deg) {
  constexpr float pi = 3.14159265358979323846f;
  return deg * (pi / 180.0f);
}

float BootModel::computeWindYawAccel() const {
  if (m_trueWindSpeed_mps <= 0.0f || m_windHullYawAccelPerMps2 <= 0.0f) {
    return 0.0f;
  }

  // Windage-only model (no sail lift/drag):
  // crosswind component on hull creates yaw disturbance.
  const float relativeWindAngle_deg =
      normalizeAngleSignedDeg(m_trueWindDirection_deg - m_heading_deg);
  const float relativeWindAngle_rad = degToRad(relativeWindAngle_deg);

  const float windYawAccel_deg_s2 =
      m_windHullYawAccelPerMps2 * m_trueWindSpeed_mps * m_trueWindSpeed_mps *
      std::sin(relativeWindAngle_rad);

  if (!std::isfinite(windYawAccel_deg_s2)) {
    return 0.0f;
  }
  return windYawAccel_deg_s2;
}

SimulationEngine::SimulationEngine(CoreSteeringController &csc,
                                   const SimulationConfig &config)
    : m_boot(config.boat), m_csc(csc) {}

void SimulationEngine::tick(float dt, uint32_t loopTimestamp) {
  // Simulate integer compass output with nearest-degree quantization
  // (instead of truncation, which adds a systematic bias).
  const uint16_t heading_u16 = static_cast<uint16_t>(
      std::lround(m_boot.heading())) % static_cast<uint16_t>(360);
  m_csc.currentHDG(heading_u16);

  m_lastIntent = m_csc.tick(loopTimestamp);

  if (m_lastIntent.has_value()) {
    const float magnitude =
        static_cast<float>(m_lastIntent->abstractImpulse_0_100);
    const float signedImpulse =
        (m_lastIntent->dir == SteeringDirection::Left) ? -magnitude : magnitude;
    m_boot.applyIntent(signedImpulse);
  }
  m_boot.update(dt);
}

float SimulationEngine::heading() const { return m_boot.heading(); }

float SimulationEngine::angularVelocity() const {
  return m_boot.angularVelocity();
}

float SimulationEngine::tillerPosition() const { return m_boot.tillerPosition(); }

float SimulationEngine::tillerYawAccel() const { return m_boot.tillerYawAccel(); }

float SimulationEngine::windYawAccel() const { return m_boot.windYawAccel(); }

float SimulationEngine::dampingYawAccel() const {
  return m_boot.dampingYawAccel();
}

std::optional<SteeringIntent> SimulationEngine::lastIntent() const {
  return m_lastIntent;
}

int main() {
  SimulationConfig config = makeDefaultSimulationConfig();
  if (!loadSimulationConfigFromJson(kNextRunConfigPath, config)) {
    return 1;
  }

  if (!ensureOutputDirectoryExists()) {
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
       << "tiller_position,"
       << "tiller_yaw_accel_deg_s2,"
       << "wind_yaw_accel_deg_s2,"
       << "damping_yaw_accel_deg_s2,"
       << "target_deg,"
       << "error_deg,"
       << "median_deg,"
       << "sample_size,"
       << "deadband_active,"
       << "obs_blocked,"
       << "intent_blocked,"
       << "intent,"
       << "intent_abstract_force,"
       << "dir\n";

  file << std::fixed << std::setprecision(3);

  uint32_t time_ms = 0;

  for (uint32_t i = 0; i < steps; ++i) {
    engine.tick(config.dt_sec, time_ms);
    const auto intent = engine.lastIntent();
    const float heading = engine.heading();
    const float angularVelocity = engine.angularVelocity();
    const float tillerPosition = engine.tillerPosition();
    const float tillerYawAccel = engine.tillerYawAccel();
    const float windYawAccel = engine.windYawAccel();
    const float dampingYawAccel = engine.dampingYawAccel();

    const auto &dbg = csc.getDebug();

    file << time_ms << "," << heading << "," << angularVelocity << ","
         << tillerPosition << "," << tillerYawAccel << "," << windYawAccel
         << "," << dampingYawAccel << "," << config.target_deg << ","
         << dbg.error << "," << dbg.median << ","
         << static_cast<int>(dbg.sampleSize) << "," << dbg.deadbandActive << ","
         << dbg.observationBlocked << "," << dbg.intentBlocked << ","
         << (intent.has_value() ? 1 : 0) << ","
         << (intent.has_value()
                 ? static_cast<int>(intent->abstractImpulse_0_100)
                 : 0)
         << ",";

    if (intent) {
      file << (intent->dir == SteeringDirection::Left ? "Left" : "Right");
    }

    file << "\n";

    time_ms += dt_ms;
  }

  file.close();
  return 0;
}
