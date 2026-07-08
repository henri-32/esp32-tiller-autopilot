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
  QueueBundle_t get_gps_bundle()const;
  QueueHandle_t get_nmea_handle()const;
  QueueHandle_t get_source_log_handle() const;

private: 
  QueueHandle_t gpsDataQueue_; 
  QueueHandle_t gpsPerformanceQueue_;
  QueueHandle_t gpsErrorQueue_;
  QueueHandle_t gpsNMEAQueue_;
  QueueHandle_t sourceLogMessageQueue_;
};
