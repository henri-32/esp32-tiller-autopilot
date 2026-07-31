#pragma once
#include "core/freertosTypes.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"

class QueueServer
{
public:
  QueueServer() = default;
  ~QueueServer() = default;

  BaseType_t init();
  QueueBundle_t get_telemetry_bundle() const;

  QueueHandle_t get_nmea_handle() const;

  QueueHandle_t get_telemetry_log_handle() const;

private:
  QueueHandle_t dataQueue_;
  QueueHandle_t runtimeLogQueue_;
  QueueHandle_t errorQueue_;

  QueueHandle_t gpsNMEAQueue_;

  QueueHandle_t telemetryLogMessageQueue_;

};
