#pragma once
#include "config/gps_hwconfig.h"
#include "esp_err.h"

struct GpsData
{
  uint32_t latitude = 0;
  uint32_t longitude = 0;
  float speed_kts = 0;
  uint16_t course_true = 0;
  uint8_t fixQuality = 0;
  uint8_t satellites_tracked = 0;
};

class IGpsDriver
{
public:
  virtual esp_err_t init() = 0;
};

class GpsDriver : public IGpsDriver
{
public:
  explicit GpsDriver() = default;

  esp_err_t init() override;

private:
  const char* TAG = "GpsDriver";
  const char* raw_msg();
};

//=================================================================
class full_sentence
{
public:
  full_sentence()
  {
    uart_data_ = static_cast<uint8_t*>(pvPortMalloc(GpsConfig::RX_buffer));
    sentence_ = static_cast<char*>(pvPortMalloc(GpsConfig::max_sentence_len));
    gpsData_ = static_cast<GpsData*>(pvPortMalloc(sizeof(GpsData)));
  };

  ~full_sentence()
  {
    vPortFree(uart_data_);
    vPortFree(sentence_);
    vPortFree(gpsData_);
  };

  const char* get_sentence();
  void parse_sentence();

private:
  void read_buffer();
  void consume_uart_data();
  void consume_byte(char byte);

  uint8_t* uart_data_;
  uint16_t bytes_read_;
  uint16_t sentence_len_ = 0;
  bool sentence_compl_ = true;
  char* sentence_;
  const char* last_compl_sentence_ = {};
  GpsData* gpsData_;
};
