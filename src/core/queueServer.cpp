#include "core/queueServer.h"
#include "types/sensorTypes.h"

BaseType_t QueueServer::init()
{
  gpsDataQueue_ = xQueueCreate(1, sizeof(Data));
  gpsPerformanceQueue_ = xQueueCreate(1, sizeof(Performance));

  // TODO Size of Error unbekannt
  gpsErrorQueue_ = xQueueCreate(1, sizeof(uint16_t));

  if (gpsDataQueue_ != nullptr && gpsPerformanceQueue_ != nullptr && gpsErrorQueue_ != nullptr)
  {
    return pdPASS;
  }
  else
  {
    return pdFAIL;
  }
};

QueueBundle QueueServer::get_gps_bundle()
{
  return {
      .data = gpsDataQueue_,
      .performance = gpsPerformanceQueue_,
      .error = gpsErrorQueue_,
  };
}
