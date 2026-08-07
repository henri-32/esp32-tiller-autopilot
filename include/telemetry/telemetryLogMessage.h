#pragma once

#include "navigation/navigationTypes.h"
#include "telemetry/runtimeTypes.h"

struct TelemetryError
{
  bool validFix = false;
};

struct TelemetryRuntimeLog
{
  RuntimeLog_t gps;
  RuntimeLog_t tHub;
  RuntimeLog_t inputHandler;
};

struct AccumulatedLogMessage
{
  NavigationSnapshot_t snapshot;
  TelemetryError error;
  TelemetryRuntimeLog runtime_log;
};


