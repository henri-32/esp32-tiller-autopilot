#include "drivers/gpsDriver.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "minmea/minmea.h"
#include "utils/debug_utils.h"

void vGpsTask(void* pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(GpsConfig::task_period);

  while (true)
  {
    task_context* context = static_cast<task_context*>(pvParameters);
    GpsDriver* driver = static_cast<GpsDriver*>(context->THIS);

    driver->fill_data();
    driver->gpsTelemetry_->rl.free_task_stack = uxTaskGetStackHighWaterMark(nullptr);

    xQueueOverwrite(context->queueBundle.data, &driver->gpsTelemetry_->data);
    xQueueOverwrite(context->queueBundle.runtime_log, &driver->gpsTelemetry_->rl);

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
  vTaskDelete(nullptr);
}

esp_err_t GpsDriver::init()
//{{{
{
  esp_err_t install = uart_driver_install(GpsConfig::uart_num, GpsConfig::RX_buffer,
                                          GpsConfig::TX_buffer, GpsConfig::event_queue_size,
                                          GpsConfig::uart_queue, GpsConfig::interrupt_alloc_flags);
  ESP_LOGI(TAG, "uart_driver_install(): %s", esp_err_to_name(install));

  uart_config_t uart_config = {
      .baud_rate = GpsConfig::baud_rate,
      .data_bits = GpsConfig::data_bits,
      .parity = GpsConfig::parity,
      .stop_bits = GpsConfig::stop_bits,
      .flow_ctrl = GpsConfig::flow_ctrl,
  };

  esp_err_t configure = uart_param_config(GpsConfig::uart_num, &uart_config);

  esp_err_t set_pin = uart_set_pin(GpsConfig::uart_num, GpsConfig::TX_GPIO, GpsConfig::RX_GPIO,
                                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  context_->THIS = this;
  context_->queueBundle = qServer_->get_gps_bundle();
  nmea_handle_ = qServer_->get_nmea_handle();

  const char* description = "GpsTask";

  BaseType_t task_create;

  if (context_->queueBundle.data != nullptr)
  {
    task_create = xTaskCreate(vGpsTask, description, 10000, context_, 5, nullptr);
  }

  RuntimeLog_t rl{};

  if (install != ESP_OK || configure != ESP_OK || set_pin != ESP_OK || task_create != pdPASS)
  {
    rl.init_status = static_cast<uint8_t>(InitStatus::OK);
    xQueueSend(context_->queueBundle.runtime_log, &rl, pdMS_TO_TICKS(100));

    return ESP_FAIL;
  }
  else
  {
    rl.init_status = static_cast<uint8_t>(InitStatus::FAIL);
    xQueueSend(context_->queueBundle.runtime_log, &rl, pdMS_TO_TICKS(100));
    return ESP_OK;
  }
}
//}}}

void GpsDriver::fill_data()
//{{{
{
  if (uart_data_ == nullptr)
  {
    return;
  }

  bytes_read_ = uart_read_bytes(GpsConfig::uart_num, uart_data_, GpsConfig::RX_buffer,
                                20 / portTICK_PERIOD_MS);
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

    // Write into queue for direct use by the logger/OpenCPN bridge.
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
        gpsTelemetry_->data.fixQuality = frame.fix_quality;
        gpsTelemetry_->data.satellites_tracked = frame.satellites_tracked;
        gpsTelemetry_->data.latitude = minmea_tocoord(&frame.latitude);
        gpsTelemetry_->data.longitude = minmea_tocoord(&frame.longitude);
      }
      break;
    }
    case MINMEA_SENTENCE_VTG:
    {
      minmea_sentence_vtg frame{};
      if (minmea_parse_vtg(&frame, sentence))
      {
        gpsTelemetry_->data.course_true = minmea_tofloat(&frame.true_track_degrees);
        gpsTelemetry_->data.speed_kts = minmea_tofloat(&frame.speed_knots);
        gpsTelemetry_->data.timestamp = esp_timer_get_time();
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
