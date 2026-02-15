#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>

#include "core/config.h"
#include "core/steering/csc/csc.h"
#include "headingSimulator.h"
#include "types/globalTypes.h"


int main() {

  constexpr double dt_sec = 0.5; // 2 Hz
  constexpr uint32_t dt_ms = 500;
  constexpr uint32_t sim_seconds = 180;

  uint32_t steps = static_cast<uint32_t>(sim_seconds / dt_sec);

  // ----------------------------
  // CSC Config
  // ----------------------------
  SteeringRegulationConfig reg{};
  reg.steeringTolerance_deg = 5;
  reg.minimumSampleSize = 5;
  reg.minimumTimeBtwObs_ms = 500; // matches 4 Hz
  reg.pauseForValidObsAfterImpulse_ms = 3000;
  reg.steeringCooldown_ms = 2000;

  CoreSteeringController csc(reg);

  uint16_t target = 100;
  csc.setInternalTarget(target);

  HeadingDriftSignal signal(100.0, 0.2); // 0.2 deg/s drift

  // ----------------------------
  // CSV
  // ----------------------------
  constexpr const char *kOutputPath = "sim/sim_output/drift_sim.csv";
  std::ofstream file(kOutputPath);
  if (!file.is_open()) {
    std::cerr << "Failed to open simulation output: " << kOutputPath << "\n";
    return 1;
  }

  file << "time_ms,"
       << "heading_deg,"
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

    signal.step(dt_sec);

    double heading = signal.getHeading();
    uint16_t hdg_u16 = static_cast<uint16_t>(heading) % 360;

    csc.currentHDG(hdg_u16);
    auto intent = csc.tick(time_ms);

    const auto &dbg = csc.getDebug();

    file << time_ms << "," << heading << "," << target << "," << dbg.error
         << "," << dbg.median << "," << static_cast<int>(dbg.sampleSize) << ","
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

