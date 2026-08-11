#pragma once
#include "navigation/navigationTypes.h"
#include "telemetry/telemetryLogMessage.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <string>
#include <unistd.h>

inline void log_raw_from_ALM(AccumulatedLogMessage msg)
{

  int fd = open("/tmp/autopilot_ALM.log", O_WRONLY | O_CREAT | O_APPEND, 0644);

  static int message_count = 0;
  dprintf(fd, "Received Message Number: %d\n", message_count);

  std::time_t now = std::time(nullptr);
  std::tm local_time{};
  localtime_r(&now, &local_time);
  char timestamp_buffer[64];
  std::strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%d %H:%M:%S", &local_time);

  std::string timestamp(timestamp_buffer);
  timestamp.append("\n");
  dprintf(fd, "%s \n", timestamp.data());

  // ======================================== Snapshot ==========================================
  dprintf(fd, "-SNAPSHOT-\n");
  auto s = msg.snapshot;
  dprintf(fd, "TARGET: %d \n", s.target_course);
  dprintf(fd, "HDG: %d \n", s.compass_hdg_dg.value);
  dprintf(fd, "COG: %d \n", s.gps_cog_dg.value);
  dprintf(fd, "SOG: %f \n", s.gps_sog_kts.value);
  dprintf(fd, "LAT: %f raw \n", s.gps_lat);
  dprintf(fd, "LON: %f raw \n", s.gps_lon);
  dprintf(fd, "LEAD_SOURCE: %s \n", navigationTypes::to_string(s.lead_source));
  dprintf(fd, "STEERING: %s \n", navigationTypes::to_string(s.steering_engaged));

  dprintf(fd, "\n");

  // ========================================= Error ============================================
  dprintf(fd, "-ERROR-\n");
  auto e = msg.error;
  dprintf(fd, "FIX: %s \n", AML::to_string(e.validFix));

  dprintf(fd, "\n");

  // ========================================= RuntimeLog =======================================
  dprintf(fd, "-RUNTIMELOG-\n");
  auto r = msg.runtime_log;
  dprintf(fd, R"(GPS free task stack: %zu
)", r.gps.free_task_stack);

  dprintf(fd, R"(THUB free task stack: %zu
)", r.tHub.free_task_stack);

  dprintf(fd, R"(INPUTHANDLER free task stack: %zu
)", r.inputHandler.free_task_stack);
  dprintf(fd, "\n");
  dprintf(fd, "\n\n");

  message_count++;
  close(fd);
};
