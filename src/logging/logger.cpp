#include "logging/logger.h"
#include "cobs-c/cobs.h"
#include "driver/uart.h"
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
  const uint16_t uart_buffersize = 2048;

  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
  };
  uart_param_config(UART_NUM_0, &uart_config);

  uart_driver_install(UART_NUM_0, uart_buffersize, 0, 0, nullptr, 0);

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

  uint8_t nav_msg_bytes[MessageOffsets::payload + NavigationPayloadOffsets::payload_length];
  uint16_t nav_msg_size = mp_write_NavigationMessage_to_bytes(nav_msg_bytes, snapshot);
  uint8_t nav_msg_encoded[nav_msg_size + (nav_msg_size / 256) + 1];

  cobs_encode_result enc_nav_res =
      cobs_encode(static_cast<void*>(&nav_msg_encoded), sizeof(nav_msg_encoded),
                  static_cast<const void*>(&nav_msg_bytes), nav_msg_size);

  if (enc_nav_res.status == COBS_ENCODE_OK)
  {
    uart_write_bytes(UART_NUM_0, nav_msg_encoded, enc_nav_res.out_len);
    write_delimiter();
  }

  // Log raw NMEA
  // =====================================================================================

  Message<NmeaSentences> nmea_msg{&nmea_sentences_};
  uint8_t nmea_msg_bytes[MessageOffsets::payload + NmeaPayloadOffsets::sentences +
                         sizeof(nmea_sentences_.sentence)];

  uint16_t size = mp_write_NmeaMessage_to_bytes(&nmea_msg_bytes, &nmea_msg);

  uint8_t nmea_msg_encoded[size + size / 256 + 1];
  cobs_encode_result enc_nmea =
      cobs_encode(static_cast<void*>(&nmea_msg_encoded), sizeof(nmea_msg_encoded),
                  static_cast<const void*>(&nmea_msg_bytes), size);

  if (enc_nmea.status == COBS_ENCODE_OK)
  {
    uart_write_bytes(UART_NUM_0, nmea_msg_encoded, enc_nmea.out_len);
    write_delimiter();
  }
};

void Logger::write_delimiter()
{
  const uint8_t delimiter = 0x00;
  uart_write_bytes(UART_NUM_0, &delimiter, sizeof(delimiter));
};
