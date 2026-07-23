#include "logging/logger.h"
#include "drivers/uartDriver.h"
#include "logging/message_protocol.h"

void vLogSourceAndPerformanceTask(void* pvParameters)
{
  task_context* context = static_cast<task_context*>(pvParameters);
  Logger* logger = reinterpret_cast<Logger*>(context->THIS);

  if (logger != nullptr)
  {
    while (1)
    {
      logger->readQueues();
      logger->log();

      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
};

BaseType_t Logger::init()
//{{{
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
//}}}

void Logger::readQueues()
//{{{
{
  BaseType_t nmea_msg = xQueuePeek(src_msg_handle_, &src_msg_, 0);
  if (nmea_msg == pdTRUE)
  {
    src_msg_recieved_ = true;
  }

  nmea_sentences_.sentence_count = 0;
  while (nmea_handle_ != nullptr && nmea_sentences_.sentence_count < NmeaConfig::queue_depth &&
         xQueueReceive(nmea_handle_, nmea_sentences_.sentence[nmea_sentences_.sentence_count], 0) ==
             pdTRUE)
  {
    ++nmea_sentences_.sentence_count;
  }
};
//}}}

void Logger::log()
{
  // TODO Es dürfte Sinn machen die Daten auf dem heap zu speichern

  // Delimiter to write after complete Message

  // Log Navigation Snapshot
  // ===========================================================================
  NavigationSnapshot snapshot;
  snapshot.gps_cog_dg.value = src_msg_.data.gps_cog.value;
  snapshot.gps_cog_dg.valid = src_msg_.data.gps_cog.valid;
  snapshot.gps_sog_kts.value = src_msg_.data.gps_sog.value;
  snapshot.gps_sog_kts.valid = src_msg_.data.gps_sog.valid;

  Message<NavigationSnapshot> nav_msg{&snapshot};
  uint8_t nav_msg_bytes[MessageOffsets::payload + NavigationPayloadOffsets::payload_length];
  uint16_t nav_msg_size = mp_write_NavigationMessage_to_bytes(nav_msg_bytes, &nav_msg);

  uartDriver_->write(uartInterface::USB_OUT, nav_msg_bytes, nav_msg_size);

  // Log raw NMEA
  // =====================================================================================

  Message<NmeaSentences> nmea_msg{&nmea_sentences_};
  uint8_t nmea_msg_bytes[MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                         sizeof(nmea_sentences_.sentence)];

  uint16_t size = mp_write_NmeaMessage_to_bytes(&nmea_msg_bytes, &nmea_msg);

  uartDriver_->write(uartInterface::USB_OUT, nmea_msg_bytes, size);
};

