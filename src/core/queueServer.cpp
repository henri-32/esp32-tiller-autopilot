#include "core/queueServer.h"
#include "core/dataStore.h"
#include "types/sensorTypes.h"
#include "types/loggingTypes.h"

BaseType_t QueueServer::init()
{
  gpsDataQueue_ = xQueueCreate(1, sizeof(GpsData));
  gpsPerformanceQueue_ = xQueueCreate(1, sizeof(PerformanceData));

  // TODO Size of Error unbekannt
  gpsErrorQueue_ = xQueueCreate(1, sizeof(uint16_t));
  gpsNMEAQueue_ = xQueueCreate(NmeaConfig::queue_depth, NmeaConfig::max_sentence_len);

  sourceLogMessageQueue_ = xQueueCreate(1, sizeof(SourceLogMessage));

  if (gpsDataQueue_ != nullptr && gpsPerformanceQueue_ != nullptr && gpsErrorQueue_ != nullptr &&
      gpsNMEAQueue_ != nullptr && sourceLogMessageQueue_ != nullptr)
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

QueueHandle_t QueueServer::get_nmea_handle() const {
  return gpsNMEAQueue_;
};

QueueHandle_t QueueServer::get_source_log_handle() const
{
  return sourceLogMessageQueue_;
};
