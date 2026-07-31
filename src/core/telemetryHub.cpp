#include "core/telemetryHub.h"
#include "config/config.h"
#include "drivers/gpsTypes.h"
#include "protocol/internalMessageProtocol.h"

void vTelemetryHubTask(void* pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(50);

  while (true)
  {
    TelemetryHub* hub = static_cast<TelemetryHub*>(pvParameters);

    hub->collectTelemetry();
    hub->collectRuntimeLogs();
    hub->updateOwnFreeStack();
    hub->publishTelemetry();

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
  vTaskDelete(nullptr);
};

BaseType_t TelemetryHub::init()
{
  BaseType_t create_res = pdFAIL;
  telemetry_bundle_ = qServer_->get_telemetry_bundle();
  telemetry_log_queue_ = qServer_->get_telemetry_log_handle();

  if (telemetry_bundle_.telemetry_msg != nullptr && telemetry_log_queue_ != nullptr)
  {
    log_msg_.runtime_log.tHub.init_status = static_cast<uint8_t>(InitStatus::OK);
    create_res = xTaskCreate(vTelemetryHubTask, "TelemetryHub", 50000, THIS, 4, nullptr);
  }

  if (create_res == pdPASS)
  {
    return pdPASS;
  }
  else
  {
    log_msg_.runtime_log.tHub.init_status = static_cast<uint8_t>(InitStatus::FAIL);
    if (telemetry_log_queue_ != nullptr)
    {
      xQueueSend(telemetry_log_queue_, &log_msg_, pdMS_TO_TICKS(100));
    }
    return pdFALSE;
  }
};

void TelemetryHub::collectTelemetry()
{

  TelemetryMessage frame{};
  xQueueReceive(telemetry_bundle_.telemetry_msg, &frame, 0);

  switch (frame.id)
  {

  case ModuleID::GPS:
  {
    {
      //==========Validity==========================
      bool gps_valid = false;
      if (frame.payload.gpsData.fixQuality == 0)
      {
        gps_valid = false;
      }
      else
      {
        gps_valid = true;
      }

      //========= Telemetry ======================
      log_msg_.snapshot.gps_sog_kts.value = frame.payload.gpsData.speed_kts;
      log_msg_.snapshot.gps_cog_dg.value = frame.payload.gpsData.course_true;
      log_msg_.snapshot.gps_lat = frame.payload.gpsData.latitude;
      log_msg_.snapshot.gps_lon = frame.payload.gpsData.longitude;

      if (gps_valid &&
          log_msg_.snapshot.gps_sog_kts.value > SteeringSourceHandlingConfig::minGpsSpeedForUse)
      {
        log_msg_.snapshot.gps_sog_kts.valid = true;
        log_msg_.snapshot.gps_cog_dg.valid = true;
      }
      else
      {
        log_msg_.snapshot.gps_sog_kts.valid = false;
        log_msg_.snapshot.gps_cog_dg.valid = false;
      }

      log_msg_.snapshot.gps_sog_kts.timestamp = frame.payload.gpsData.timestamp;
      log_msg_.snapshot.gps_cog_dg.timestamp = frame.payload.gpsData.timestamp;
      log_msg_.error.validFix = gps_valid;
    }
    break;
  }

  case ModuleID::INPUT_HANDLER:
  {
    log_msg_.snapshot.steering_engaged = frame.payload.inputHandleData.steering_engaged;
	log_msg_.snapshot.target_course = frame.payload.inputHandleData.target_course;
  }
  }
};

//==========================================================================================================
//==========================================================================================================

void TelemetryHub::collectRuntimeLogs()
{
  RuntimeLogMessage frame{ModuleID::THUB};
  xQueueReceive(telemetry_bundle_.runtime_log, &frame, 0);

  switch (frame.id)
  {
  case ModuleID::GPS:
  {
    log_msg_.runtime_log.gps.free_task_stack = frame.payload.free_task_stack;
    log_msg_.runtime_log.gps.init_status = frame.payload.init_status;
    break;
  }

  case ModuleID::INPUT_HANDLER:
  {
    log_msg_.runtime_log.inputHandler.free_task_stack = frame.payload.free_task_stack;
    log_msg_.runtime_log.inputHandler.init_status = frame.payload.init_status;
    break;
  }
  case ModuleID::THUB:
    break;
  };
};

void TelemetryHub::publishTelemetry()
{

  xQueueOverwrite(telemetry_log_queue_, &log_msg_);
};

void TelemetryHub::updateOwnFreeStack()
{
  log_msg_.runtime_log.tHub.free_task_stack = uxTaskGetStackHighWaterMark(nullptr);
};
