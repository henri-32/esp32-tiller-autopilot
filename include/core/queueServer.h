#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "types/systemServiceTypes.h"

class QueueServer
{
public:
  QueueServer() = default;
  ~QueueServer() = default;

  BaseType_t init();
  QueueBundle get_gps_bundle();

private: 
  QueueHandle_t gpsDataQueue_; 
  QueueHandle_t gpsPerformanceQueue_;
  QueueHandle_t gpsErrorQueue_;
};
