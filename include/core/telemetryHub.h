#pragma once
#include "core/queueServer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "types/sensorTypes.h"
#include "types/systemServiceTypes.h"
#include <new>

struct TelemetrySnapshot
{
  SensorSample<uint16_t> gps_cog{.value = 0};
  SensorSample<float> gps_sog{.value = 0.0F};
  float gps_lat = 0.0F; 
  float gps_lon = 0.0F; 
};

struct TelemetryError
{
  bool validFix = false;
};

struct TelemetryPerformance
{
  uint16_t gpsFreeStack = 0;
  uint16_t telemetryHubFreeStack = 0;
};

struct TelemetryLogMessage
{
  TelemetrySnapshot snapshot;
  TelemetryPerformance performance;
};

class TelemetryHub
{
public:
  TelemetryHub(const QueueServer* const qServer)
  {
    qServer_ = qServer;

    void* contextMem = pvPortMalloc(sizeof(task_context));
    if (contextMem != nullptr)
    {
      context_ = new (contextMem) task_context{};
    }
  };

  ~TelemetryHub()
  {
    if (context_ != nullptr)
    {
      vPortFree(context_);
    }
  };

  BaseType_t init();
  void collectTelemetry();
  void publishTelemetry();
  void updateOwnFreeStack();

private:
  task_context* context_;
  const QueueServer* qServer_;
  QueueBundle_t gps_bundle_;
  QueueHandle_t telemetry_log_queue_;
  TelemetrySnapshot telemetrySnapshot_;
  TelemetryError telemetryError_;
  TelemetryPerformance telemetryPerformance_;
};
