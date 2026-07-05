#include "drivers/gpsDriver.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "utils/debug_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "minmea.h"

void vGpsTask(void* pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(GpsConfig::task_period);

  while (true)
  {
    gps::task_context* context = static_cast<gps::task_context*>(pvParameters);
    GpsDriver* driver = static_cast<GpsDriver*>(context->THIS);
    driver->fill_data();
    xQueueOverwrite(context->gpsQueue, driver->gpsData_);
	WRITE_FREE_TASK_STACK_TO_CONTEXT(context);

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
      .baud_rate = 3840,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };

  esp_err_t configure = uart_param_config(GpsConfig::uart_num, &uart_config);

  esp_err_t set_pin = uart_set_pin(GpsConfig::uart_num, GpsConfig::TX_GPIO, GpsConfig::RX_GPIO,
                                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  context_->gpsQueue = xQueueCreate(1, sizeof(gps::data));
  context_->THIS = this;

  const char* description =
      "Reading Gps Data over UART, parsing the data and sending it to the gpsQueue";

  BaseType_t task_create;

  if (context_->gpsQueue != nullptr)
  {
    task_create = xTaskCreate(vGpsTask, description, 10000, context_, 5, nullptr);
  }

  if (install != ESP_OK || configure != ESP_OK || set_pin != ESP_OK || task_create != pdPASS)
  {
    return ESP_FAIL;
  }
  else
  {
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

void GpsDriver::parse_sentence(const char* sentence)
//{{{
{
  if (sentence == nullptr)
  {
    return;
  }

  if (minmea_check(sentence, true))

  {
    const enum minmea_sentence_id id = minmea_sentence_id(sentence, true);
    switch (id)
    {
    case MINMEA_SENTENCE_GGA:
    {
      minmea_sentence_gga frame{};
      if (minmea_parse_gga(&frame, sentence))
      {
        gpsData_->fixQuality = frame.fix_quality;
        gpsData_->satellites_tracked = frame.satellites_tracked;
        gpsData_->latitude = minmea_tocoord(&frame.latitude);
        gpsData_->longitude = minmea_tocoord(&frame.longitude);
      }
      break;
    }
    case MINMEA_SENTENCE_VTG:
    {
      minmea_sentence_vtg frame{};
      if (minmea_parse_vtg(&frame, sentence))
      {
        gpsData_->course_true = minmea_tofloat(&frame.true_track_degrees);
        gpsData_->speed_kts = minmea_tofloat(&frame.speed_knots);
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

  // sentence_compl_ garantees that cut off sentences get discarded // $ marks beginning of nmea
  // sentence
  if (sentence_compl_ && byte == '$')
  {
    // starting the sentence with $ and marking the sentence incomplete
    sentence_len_ = 0;
    sentence_[sentence_len_] = byte;
    sentence_len_++;
    sentence_compl_ = false;
  }

  //'\n' marks the end of nmea sentence
  else if (byte == '\n')
  {
    // marking the sentence complete, adding nullterminator and parsing it to the gpsData_ struct
    sentence_compl_ = true;
    sentence_[sentence_len_] = 0x00;
    parse_sentence(sentence_);
  }

  // normal characters
  else
  {
    // normal characters btw. sentences have to be noise
    if (!sentence_compl_)
    {
      // Limiting sentence_len_ to prevent crashes if start byte gets decoded from garbage bytes
      if (sentence_len_ < 2000)
      {
        // normal characters get written into the sentence
        sentence_[sentence_len_] = byte;
        sentence_len_++;
      }
      // Limiting sentence_len_
      else
      {
        sentence_compl_ = true;
        sentence_len_ = 0;
      }
    }
  }
}
//}}}
