#pragma once
#include "core/dataStore.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "types/loggingTypes.h"
#include "types/systemServiceTypes.h"
#include <new>

class Logger

{
public:
  Logger(QueueServer* const qServer) : qServer_(qServer)
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
  task_context* context_;
  QueueHandle_t src_msg_handle_;
  QueueHandle_t nmea_handle_;
  SourceLogMessage src_msg_{};
  char nmea_sentences_[NmeaConfig::queue_depth][NmeaConfig::max_sentence_len]{};
  uint8_t nmea_sentence_count_ = 0;
  bool src_msg_recieved_ = false;
  bool log_to_esp_usb_flag = true;
};
