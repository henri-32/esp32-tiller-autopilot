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
  QueueHandle_t get_telemetry_log_handle() const;
  QueueBundle_t get_input_handle() const;

private: 
  QueueHandle_t gpsDataQueue_; 
  QueueHandle_t gpsRuntimeLogQueue_;
  QueueHandle_t gpsErrorQueue_;
  QueueHandle_t gpsNMEAQueue_;
  QueueHandle_t telemetryLogMessageQueue_;
  QueueHandle_t inputHandlerDataQueue_;
  QueueHandle_t inputErrorQueue_;
  QueueHandle_t inputHandlerRuntimeLogQueue_;
};
