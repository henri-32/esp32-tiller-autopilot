#include "logging/logger.h"
#include "config/nmeaConfig.h"
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

  BaseType_t task = pdFALSE;
  if (context_ != nullptr)
  {
    context_->THIS = this;
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
  telemetry_log_message_received_ =
      telemetry_log_handle_ != nullptr &&
      xQueuePeek(telemetry_log_handle_, &telemetry_log_message_, 0) == pdTRUE;

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
  if (telemetry_log_message_received_)
  {
    Message<AccumulatedLogMessage> telemetry_msg{&telemetry_log_message_};
    uint8_t telemetry_msg_bytes[MessageOffsets::payload +
                                AccumulatedLogMessagePayloadOffsets::payload_length];
    const uint16_t telemetry_msg_size = mp_write_AccumulatedLogMessage_to_bytes(
        telemetry_msg_bytes, sizeof(telemetry_msg_bytes), &telemetry_msg);
    if (telemetry_msg_size > 0)
    {
      uartDriver_->write(UartInterface::GATEWAY, telemetry_msg_bytes, telemetry_msg_size);
    }
  }

  // Log raw NMEA
  if (nmea_sentences_.sentence_count > 0)
  {
    Message<NmeaSentences> nmea_msg{&nmea_sentences_};
    uint8_t nmea_msg_bytes[MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                           sizeof(nmea_sentences_.sentence)];
    const uint16_t size =
        mp_write_NmeaMessage_to_bytes(nmea_msg_bytes, sizeof(nmea_msg_bytes), &nmea_msg);
    if (size > 0)
    {
      uartDriver_->write(UartInterface::GATEWAY, nmea_msg_bytes, size);
    }
  }
};
