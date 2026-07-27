#include "logging/logger.h"
#include "drivers/uartDriver.h"
#include "protocol/autopilotWireProtocol.h"

void vTelemetryLoggingTask(void* pvParameters)
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
  telemetry_log_handle_ = qServer_->get_telemetry_log_handle();
  nmea_handle_ = qServer_->get_nmea_handle();

  context_->THIS = this;

  BaseType_t task;
  if (context_ != nullptr)
  {
    task = xTaskCreate(vTelemetryLoggingTask, "Logging", 10000, context_, 1, nullptr);
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
  BaseType_t telemetry_message = xQueuePeek(telemetry_log_handle_, &telemetry_log_message_, 0);
  if (telemetry_message == pdTRUE)
  {
    telemetry_log_message_received_ = true;
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
  snapshot.gps_cog_dg.value = telemetry_log_message_.snapshot.gps_cog.value;
  snapshot.gps_cog_dg.valid = telemetry_log_message_.snapshot.gps_cog.valid;
  snapshot.gps_sog_kts.value = telemetry_log_message_.snapshot.gps_sog.value;
  snapshot.gps_sog_kts.valid = telemetry_log_message_.snapshot.gps_sog.valid;

  Message<NavigationSnapshot> nav_msg{&snapshot};
  uint8_t nav_msg_bytes[MessageOffsets::payload + NavigationPayloadOffsets::payload_length];
  uint16_t nav_msg_size = mp_write_NavigationMessage_to_bytes(nav_msg_bytes,sizeof(nav_msg_bytes),  &nav_msg);

  uartDriver_->write(UartInterface::USB_INTERFACE, nav_msg_bytes, nav_msg_size);

  // Log raw NMEA
  // =====================================================================================

  Message<NmeaSentences> nmea_msg{&nmea_sentences_};
  uint8_t nmea_msg_bytes[MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                         sizeof(nmea_sentences_.sentence)];

  uint16_t size = mp_write_NmeaMessage_to_bytes(nmea_msg_bytes, sizeof(nmea_msg_bytes), &nmea_msg);

  uartDriver_->write(UartInterface::USB_INTERFACE, nmea_msg_bytes, size);
};
