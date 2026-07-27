#include "core/telemetryHub.h"
#include "config/config.h"

void vTelemetryHubTask(void* pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(50);

  while (true)
  {
    task_context* context = static_cast<task_context*>(pvParameters);
    TelemetryHub* hub = static_cast<TelemetryHub*>(context->THIS);

    hub->collectTelemetry();
    hub->updateOwnFreeStack();
    hub->publishTelemetry();

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
  vTaskDelete(nullptr);
};

BaseType_t TelemetryHub::init()
{
  context_->THIS = this;
  gps_bundle_ = qServer_->get_gps_bundle();
  telemetry_log_queue_ = qServer_->get_telemetry_log_handle();

  if (gps_bundle_.data != nullptr && context_ != nullptr)
  {
    return xTaskCreate(vTelemetryHubTask, "TelemetryHub", 50000, context_, 4, nullptr);
  }
  else
  {
    return pdFALSE;
  }

  if (telemetry_log_queue_ != nullptr)
  {
    return pdTRUE;
  }
  else
  {
    return pdFALSE;
  }
};

void TelemetryHub::collectTelemetry()
{
  Telemetry<gpsDriverData_t> frame;
  xQueuePeek(gps_bundle_.data, &frame.data, 0);
  xQueuePeek(gps_bundle_.runtime_log, &frame.rl, 0);
  xQueuePeek(gps_bundle_.error, &frame.error, 0);

  //==========Errors==========================
  bool gps_valid = false;
  if (frame.data.fixQuality == 0)
  {
    gps_valid = false;
  }
  else
  {
    gps_valid = true;
  }

  //========= Telemetry ======================
  telemetrySnapshot_.gps_sog.value = frame.data.speed_kts;
  telemetrySnapshot_.gps_cog.value = frame.data.course_true;
  telemetrySnapshot_.gps_lat = frame.data.latitude;
  telemetrySnapshot_.gps_lon = frame.data.longitude;

  if (gps_valid && telemetrySnapshot_.gps_sog.value > SteeringSourceHandlingConfig::minGpsSpeedForUse)
  {
    telemetrySnapshot_.gps_sog.valid = true;
    telemetrySnapshot_.gps_cog.valid = true;
  }
  else
  {
    telemetrySnapshot_.gps_sog.valid = false;
    telemetrySnapshot_.gps_cog.valid = false;
  }

  telemetrySnapshot_.gps_sog.timestamp = frame.data.timestamp;
  telemetrySnapshot_.gps_cog.timestamp = frame.data.timestamp;

  //======== Perfomance =====================
  telemetryPerformance_.gpsFreeStack = frame.rl.free_task_stack;
};

void TelemetryHub::publishTelemetry()
{
  TelemetryLogMessage msg{};
  msg.snapshot = telemetrySnapshot_;
  msg.performance = telemetryPerformance_;

  xQueueOverwrite(telemetry_log_queue_, &msg);
};

void TelemetryHub::updateOwnFreeStack()
{
  telemetryPerformance_.telemetryHubFreeStack = uxTaskGetStackHighWaterMark(nullptr);
};
