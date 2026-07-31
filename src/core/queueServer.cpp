#include "core/queueServer.h"
#include "config/nmeaConfig.h"
#include "drivers/gpsTypes.h"
#include "telemetry/runtimeTypes.h"
#include "protocol/internalMessageProtocol.h"
#include "telemetry/telemetryLogMessage.h"

BaseType_t QueueServer::init()
{
  dataQueue_ = xQueueCreate(5, sizeof(TelemetryMessage));
  runtimeLogQueue_ = xQueueCreate(5, sizeof(RuntimeLogMessage));
  errorQueue_ = xQueueCreate(10, sizeof(ErrorMessage));

  gpsNMEAQueue_ = xQueueCreate(NmeaConfig::queue_depth, NmeaConfig::max_sentence_len);

  telemetryLogMessageQueue_ = xQueueCreate(1, sizeof(AccumulatedLogMessage));


  if (dataQueue_ != nullptr && runtimeLogQueue_ != nullptr && errorQueue_ != nullptr &&
      gpsNMEAQueue_ != nullptr && telemetryLogMessageQueue_ != nullptr)
  {
    return pdPASS;
  }
  else
  {
    return pdFAIL;
  }
};

QueueBundle_t QueueServer::get_telemetry_bundle() const
{
  return {
      .telemetry_msg = dataQueue_,
      .runtime_log = runtimeLogQueue_,
      .error = errorQueue_,
  };
}

QueueHandle_t QueueServer::get_nmea_handle() const
{
  return gpsNMEAQueue_;
};

QueueHandle_t QueueServer::get_telemetry_log_handle() const
{
  return telemetryLogMessageQueue_;
};
