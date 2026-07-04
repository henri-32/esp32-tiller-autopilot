#include "drivers/gpsDriver.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "minmea.h"

esp_err_t GpsDriver::init()
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
  ESP_LOGI(TAG, "uart_param_config(): %s", esp_err_to_name(configure));

  esp_err_t set_pin = uart_set_pin(GpsConfig::uart_num, GpsConfig::TX_GPIO, GpsConfig::RX_GPIO,
                                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  ESP_LOGI(TAG, "uart_set_pin(): %s", esp_err_to_name(set_pin));

  if (install != ESP_OK || configure != ESP_OK || set_pin != ESP_OK)
  {
    return ESP_FAIL;
  }
  else
  {
    return ESP_OK;
  }
}

const char* full_sentence::get_sentence()
{
  read_buffer();
  printf("Bytes read: %d\n", bytes_read_);
  consume_uart_data();
  // parse with minmea last_compl_sentence
  return last_compl_sentence_;
};

void full_sentence::parse_sentence()
{
  if (minmea_check(last_compl_sentence_, true))

  {
    const enum minmea_sentence_id id = minmea_sentence_id(last_compl_sentence_, true);
    switch (id)
    {
    case MINMEA_SENTENCE_GGA:
    {
      minmea_sentence_gga frame{};
      if (minmea_parse_gga(&frame, last_compl_sentence_))
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
      if (minmea_parse_vtg(&frame, last_compl_sentence_))
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

void full_sentence::read_buffer()
{
  bytes_read_ = uart_read_bytes(GpsConfig::uart_num, uart_data_, GpsConfig::RX_buffer,
                                20 / portTICK_PERIOD_MS);
}

void full_sentence::consume_uart_data()
{
  for (uint16_t i = 0; i < bytes_read_; ++i)
  {
    consume_byte(uart_data_[i]);
    // printf("%02X ", uart_data_[i]);
    printf("%c", uart_data_[i]);
  }
}

void full_sentence::consume_byte(char byte)
{
  if (sentence_compl_ && byte == '$')
  {
    sentence_len_ = 0;
    sentence_[sentence_len_] = byte;
    sentence_compl_ = false;
  }

  else if (byte == '\n')
  {
    sentence_compl_ = true;
    sentence_[sentence_len_] = 0x00;
    last_compl_sentence_ = sentence_;
  }
  else
  {
    if (!sentence_compl_)
    {
      if (sentence_len_ < 2000)
      {
        sentence_[sentence_len_] = byte;
        sentence_len_++;
      }
      else
      {
        sentence_compl_ = true;
        sentence_len_ = 0;
      }
    }
  }
}
