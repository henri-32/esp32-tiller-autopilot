#include "drivers/gpsDriver.h"
#include "drivers/uartDriver.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "minmea/minmea.h"

void vGpsTask(void* pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(500);
  GpsDriver* driver = static_cast<GpsDriver*>(pvParameters);

  while (true)
  {

    driver->fill_payload();

    TelemetryMessage tel_msg = initialize_TelemetryMessage_with_payload(driver->module_payload_);
    RuntimeLogMessage rl_msg{driver->id_};

    rl_msg.payload.free_task_stack = uxTaskGetStackHighWaterMark(nullptr);

    xQueueSend(driver->queueBundle_.telemetry_msg, &tel_msg, 0);
    xQueueSend(driver->queueBundle_.runtime_log, &rl_msg, 0);

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
  vTaskDelete(nullptr);
}

esp_err_t GpsDriver::init()
//{{{
{
  queueBundle_ = qServer_->get_telemetry_bundle();
  nmea_handle_ = qServer_->get_nmea_handle();

  if (xTaskCreate(vGpsTask, "GpsTask", 10000, this, 5, nullptr) != pdPASS)
  {
    runtimeLogMessage_.payload.init_status = static_cast<uint8_t>(InitStatus::FAIL);
    xQueueSend(queueBundle_.runtime_log, &runtimeLogMessage_, pdMS_TO_TICKS(100));
    return ESP_FAIL;
  }

  runtimeLogMessage_.payload.init_status = static_cast<uint8_t>(InitStatus::OK);
  xQueueSend(queueBundle_.runtime_log, &runtimeLogMessage_, pdMS_TO_TICKS(100));
  return ESP_OK;
}
//}}}

void GpsDriver::fill_payload()
//{{{
{
  if (uart_data_ == nullptr)
  {
    return;
  }

  // TODO remove hardcoded buffer size here and in construcor of GpsDriver
  bytes_read_ = uartDriver_->read(UartInterface::GPS, uart_data_, 1024, pdMS_TO_TICKS(50));

  consume_uart_data();
};
//}}}

void GpsDriver::consume_sentence(const char* sentence)
//{{{
{
  if (sentence == nullptr)
  {
    return;
  }

  if (minmea_check(sentence, true))

  {

    // Write into queue for direct use by the logger/gateway.
    if (nmea_handle_ != nullptr)
    {
      xQueueSendToBack(nmea_handle_, sentence, 0);
    }

    const enum minmea_sentence_id id = minmea_sentence_id(sentence, true);
    switch (id)
    {
    case MINMEA_SENTENCE_GGA:
    {
      minmea_sentence_gga frame{};
      if (minmea_parse_gga(&frame, sentence))
      {
        module_payload_.fixQuality = frame.fix_quality;
        module_payload_.satellites_tracked = frame.satellites_tracked;
        module_payload_.latitude = minmea_tocoord(&frame.latitude);
        module_payload_.longitude = minmea_tocoord(&frame.longitude);
      }
      break;
    }
    case MINMEA_SENTENCE_VTG:
    {
      minmea_sentence_vtg frame{};
      if (minmea_parse_vtg(&frame, sentence))
      {
        module_payload_.course_true = minmea_tofloat(&frame.true_track_degrees);
        module_payload_.speed_kts = minmea_tofloat(&frame.speed_knots);
        module_payload_.timestamp = esp_timer_get_time();
      }

      break;
    }

    case MINMEA_INVALID:
      break;

    case MINMEA_UNKNOWN:
      break;

    default:
      break;
    }
  };
};
//}}}

void GpsDriver::consume_uart_data()
//{{{
{
  for (uint16_t i = 0; i < bytes_read_; ++i)
  {
    consume_byte(uart_data_[i]);

    // To print raw uart_data and debug/validate parsed input.
    // printf("%c", uart_data_[i]);
  }
}
//}}}

void GpsDriver::consume_byte(char byte)
//{{{
{
  if (sentence_ == nullptr)
  {
    return;
  }

  // '$' marks the beginning of a NMEA sentence. Restarting here discards cut off sentences.
  if (byte == '$')
  {
    sentence_len_ = 0;
    sentence_[sentence_len_++] = byte;
    sentence_compl_ = false;
    return;
  }

  if (sentence_compl_)
  {
    return;
  }

  if (byte == '\r')
  {
    return;
  }

  // '\n' marks the end of a NMEA sentence.
  if (byte == '\n')
  {
    sentence_compl_ = true;
    sentence_[sentence_len_] = 0x00;
    consume_sentence(sentence_);
    sentence_len_ = 0;
    return;
  }

  // Leave room for the null terminator.
  if (sentence_len_ < GpsConfig::max_sentence_len - 1)
  {
    sentence_[sentence_len_++] = byte;
    return;
  }

  sentence_compl_ = true;
  sentence_len_ = 0;
  sentence_[0] = 0x00;
}
//}}}
