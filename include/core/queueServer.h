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
  QueueBundle_t get_input_handle() const;

private: 
  QueueHandle_t gpsDataQueue_; 
  QueueHandle_t gpsPerformanceQueue_;
  QueueHandle_t gpsErrorQueue_;
  QueueHandle_t gpsNMEAQueue_;
  QueueHandle_t sourceLogMessageQueue_;
  QueueHandle_t inputDataQueue_;
  QueueHandle_t inputPerformanceQueue_;
  QueueHandle_t inputErrorQueue_;
};
