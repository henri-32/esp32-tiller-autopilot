#include "logging/logger.h"

void vLogSourceAndPerformanceTask(void* pvParameters)
{
  task_context* context = static_cast<task_context*>(pvParameters);
  Logger* logger = reinterpret_cast<Logger*>(context->THIS);

  while (1)
  {
    logger->readQueues();
    logger->log();

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
};

BaseType_t Logger::init()
{
  src_msg_handle_ = qServer_->get_source_log_handle();
  nmea_handle_ = qServer_->get_nmea_handle();

  context_->THIS = this;

  BaseType_t task;
  if (context_ != nullptr)
  {
    task = xTaskCreate(vLogSourceAndPerformanceTask, "Logging", 10000, context_, 1, nullptr);
  }

  if (task == pdTRUE)
  {
    return pdPASS;
  }
  else
  {
    return pdFALSE;
  }
};

void Logger::readQueues()
{
  BaseType_t msg = xQueuePeek(src_msg_handle_, &src_msg_, 0);
  if (msg == pdTRUE)
  {
    src_msg_recieved_ = true;
  }

  nmea_sentence_count_ = 0;
  while (nmea_handle_ != nullptr && nmea_sentence_count_ < NmeaConfig::queue_depth &&
         xQueueReceive(nmea_handle_, nmea_sentences_[nmea_sentence_count_], 0) == pdTRUE)
  {
    ++nmea_sentence_count_;
  }
};

void Logger::log()
{
  if (log_to_esp_usb_flag != true)
  {
    return;
  }

  printf("@SOURCE &GPS -COG:%d deg -valid:%d -timestamp:%llu s\n", src_msg_.data.gps_cog.value,
         src_msg_.data.gps_cog.valid, src_msg_.data.gps_cog.timestamp / 1000);

  printf("@SOURCE &GPS -SOG:%f kts -valid:%d -timestamp:%llu s\n", src_msg_.data.gps_sog.value,
         src_msg_.data.gps_sog.valid, src_msg_.data.gps_sog.timestamp / 1000);

  printf("@SOURCE &GPS -LAT:%f dd.dd \n", src_msg_.data.gps_lat);
  printf("@SOURCE &GPS -LON:%f dd.dd \n", src_msg_.data.gps_lon);

  printf("@PERFORMANCE &GPS_TASK:%d bytes free\n", src_msg_.performance.gpsFreeStack);
  printf("@PERFORMANCE &DATASTORE_TASK:%d bytes free\n", src_msg_.performance.dataStoreFreeStack);

  for (uint8_t i = 0; i < nmea_sentence_count_; ++i)
  {
    printf("@NMEA %s\r\n", nmea_sentences_[i]);
  }

  printf("\n============================================================\n\n");
};
