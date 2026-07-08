#pragma once
#include "core/queueServer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "types/sensorTypes.h"
#include "types/systemServiceTypes.h"
#include <new>

struct DataStored
{
  SensorSample<uint16_t> gps_cog{.value = 0};
  SensorSample<float> gps_sog{.value = 0.0F};
  float gps_lat = 0.0F; 
  float gps_lon = 0.0F; 
};

struct ErrorStored
{
  bool validFix = false;
};

struct PerformanceStored
{
  uint16_t gpsFreeStack = 0;
  uint16_t dataStoreFreeStack = 0;
};

struct SourceLogMessage
{
  DataStored data;
  PerformanceStored performance;
};

class DataStore
{
public:
  DataStore(const QueueServer* const qServer)
  {
    qServer_ = qServer;

    void* contextMem = pvPortMalloc(sizeof(task_context));
    if (contextMem != nullptr)
    {
      context_ = new (contextMem) task_context{};
    }
  };

  ~DataStore()
  {
    if (context_ != nullptr)
    {
      vPortFree(context_);
    }
  };

  BaseType_t init();
  void readQueues();
  void sendQueues();
  void setOwnFreeStack();

private:
  task_context* context_;
  const QueueServer* qServer_;
  QueueBundle_t gps_bundle_;
  QueueHandle_t source_log_queue_;
  DataStored dataStored_;
  ErrorStored errorStored_;
  PerformanceStored performanceStored_;
};
