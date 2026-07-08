#include "core/dataStore.h"
#include "config/config.h"

void vDataStoreTask(void* pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(50);

  while (true)
  {
    task_context* context = static_cast<task_context*>(pvParameters);
    DataStore* store = static_cast<DataStore*>(context->THIS);

    store->readQueues();
    store->setOwnFreeStack();
    store->sendQueues();

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
  vTaskDelete(nullptr);
};

BaseType_t DataStore::init()
{
  context_->THIS = this;
  gps_bundle_ = qServer_->get_gps_bundle();
  source_log_queue_ = qServer_->get_source_log_handle();

  if (gps_bundle_.data != nullptr && context_ != nullptr)
  {
    return xTaskCreate(vDataStoreTask, "DataStore", 50000, context_, 4, nullptr);
  }
  else
  {
    return pdFALSE;
  }

  if (source_log_queue_ != nullptr)
  {
    return pdTRUE;
  }
  else
  {
    return pdFALSE;
  }
};

void DataStore::readQueues()
{
  Telemetry<GpsData> frame;
  xQueuePeek(gps_bundle_.data, &frame.data, 0);
  xQueuePeek(gps_bundle_.performance, &frame.performance, 0);
  xQueuePeek(gps_bundle_.error, &frame.error, 0);

  //==========Errors==========================
  bool gps_valid = false;
  if (frame.data.fixQuality == 0)
  {
    gps_valid = false;
  }
  else
  {
    gps_valid = true;
  }

  //========= Telemetry ======================
  dataStored_.gps_sog.value = frame.data.speed_kts;
  dataStored_.gps_cog.value = frame.data.course_true;
  dataStored_.gps_lat = frame.data.latitude;
  dataStored_.gps_lon = frame.data.longitude;

  if (gps_valid && dataStored_.gps_sog.value > SteeringSourceHandlingConfig::minGpsSpeedForUse)
  {
    dataStored_.gps_sog.valid = true;
    dataStored_.gps_cog.valid = true;
  }
  else
  {
    dataStored_.gps_sog.valid = false;
    dataStored_.gps_cog.valid = false;
  }

  dataStored_.gps_sog.timestamp = frame.data.timestamp;
  dataStored_.gps_cog.timestamp = frame.data.timestamp;

  //======== Perfomance =====================
  performanceStored_.gpsFreeStack = frame.performance.free_task_stack;
};

void DataStore::sendQueues()
{
  SourceLogMessage msg{};
  msg.data = dataStored_;
  msg.performance = performanceStored_;

  xQueueOverwrite(source_log_queue_, &msg);
};

void DataStore::setOwnFreeStack()
{
  performanceStored_.dataStoreFreeStack = uxTaskGetStackHighWaterMark(nullptr);
};
