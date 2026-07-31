#pragma once
#include "core/freertosTypes.h"
#include "core/queueServer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "telemetry/telemetryLogMessage.h"

class TelemetryHub
{
public:
  TelemetryHub(const QueueServer* const qServer) : qServer_(qServer) {};

  ~TelemetryHub() = default;

  BaseType_t init();
  void collectTelemetry();
  void collectRuntimeLogs();
  void publishTelemetry();
  void updateOwnFreeStack();

private:
  void* THIS = this;
  const QueueServer* qServer_;
  QueueBundle_t telemetry_bundle_;

  QueueHandle_t telemetry_log_queue_;
  AccumulatedLogMessage log_msg_;
};
