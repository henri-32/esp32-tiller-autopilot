#include "core/queueServer.h"
#include "core/telemetryHub.h"
#include "types/inputHandleTypes.h"
#include "types/loggingTypes.h"
#include "types/sensorTypes.h"

BaseType_t QueueServer::init()
{
  gpsDataQueue_ = xQueueCreate(1, sizeof(GpsData));
  gpsPerformanceQueue_ = xQueueCreate(1, sizeof(PerformanceData));

  // TODO Size of Error unbekannt
  gpsErrorQueue_ = xQueueCreate(1, sizeof(uint16_t));
  gpsNMEAQueue_ = xQueueCreate(NmeaConfig::queue_depth, NmeaConfig::max_sentence_len);

  telemetryLogMessageQueue_ = xQueueCreate(1, sizeof(TelemetryLogMessage));

  inputDataQueue_ = xQueueCreate(5, sizeof(InputHandleData));

  if (gpsDataQueue_ != nullptr && gpsPerformanceQueue_ != nullptr && gpsErrorQueue_ != nullptr &&
      gpsNMEAQueue_ != nullptr && telemetryLogMessageQueue_ != nullptr)
  {
    return pdPASS;
  }
  else
  {
    return pdFAIL;
  }
};

QueueBundle_t QueueServer::get_gps_bundle() const
{
  return {
      .data = gpsDataQueue_,
      .performance = gpsPerformanceQueue_,
      .error = gpsErrorQueue_,
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

QueueBundle_t QueueServer::get_input_handle() const
{
  return {
      .data = inputDataQueue_, .performance = inputPerformanceQueue_, .error = inputErrorQueue_};
};
