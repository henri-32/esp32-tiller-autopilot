#pragma once
#include "core/freertosTypes.h"
#include "core/queueServer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "protocol/autopilotWireProtocol.h"
#include "telemetry/telemetryLogMessage.h"
#include <new>

class UartDriver;

class Logger

{
public:
  Logger(QueueServer* const qServer, UartDriver* const uartDriver)
      : qServer_(qServer), uartDriver_(uartDriver)
  //{{{
  {
    void* contextMem = pvPortMalloc(sizeof(task_context));
    if (contextMem != nullptr)
    {
      context_ = new (contextMem) task_context{};
    }
  };
  //}}}

  ~Logger()
  //{{{
  {
    if (context_ != nullptr)
    {
      context_->~task_context();
    }
  };

  //}}}

  BaseType_t init();
  void readQueues();
  void log();

private:
  QueueServer* const qServer_;
  UartDriver* const uartDriver_;
  task_context* context_ = nullptr;
  QueueHandle_t telemetry_log_handle_ = nullptr;
  QueueHandle_t nmea_handle_ = nullptr;
  AccumulatedLogMessage telemetry_log_message_{};
  NmeaSentences nmea_sentences_{};
  bool telemetry_log_message_received_ = false;
  bool log_to_esp_usb_flag = true;
};
