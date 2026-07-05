#pragma once
#include "config/gps_hwconfig.h"
#include "esp_err.h"
#include "utils/debug_utils.h"
#include <new>

namespace gps {
struct data
{
  // lat and long in decimaldegree format DD.DDD...
  uint32_t latitude = 0;
  uint32_t longitude = 0;

  // SOG in knots
  float speed_kts = 0;

  // true COG
  uint16_t course_true = 0;

  // Quality of the satellite fix. 0 = invalid >0 are different types of valid (see nmea183
  // standard)
  uint8_t fixQuality = 0;

  // Number of satellites used for positioning
  uint8_t satellites_tracked = 0;

};

struct task_context
{
  void* THIS = nullptr;
  QueueHandle_t gpsQueue = nullptr;
  ADD_MEMORY_VALUES();
};
}


class IGpsDriver
//{{{
{
public:
  virtual esp_err_t init() = 0;
  ~IGpsDriver() = default;
};
//}}}

class GpsDriver : public IGpsDriver
//{{{
{
public:
  explicit GpsDriver()
  //{{{
  {
    // structs are allocated in the freeRTOS heap
    uart_data_ = static_cast<uint8_t*>(pvPortMalloc(GpsConfig::RX_buffer));
    sentence_ = static_cast<char*>(pvPortMalloc(GpsConfig::max_sentence_len));

    void* dataMem = pvPortMalloc(sizeof(gps::data));
    if (dataMem != nullptr)
    {
      gpsData_ = new (dataMem) gps::data{};
    }
    void* contextMem = pvPortMalloc(sizeof(gps::task_context));
    if (contextMem != nullptr)
    {
      context_ = new (contextMem) gps::task_context{};
    }
  }
  //}}}

  ~GpsDriver()
  //{{{
  {
    if (uart_data_ != nullptr)
    {
      vPortFree(uart_data_);
    }

    if (sentence_ != nullptr)
    {
      vPortFree(sentence_);
    }

    if (gpsData_ != nullptr)
    {
      gpsData_->~data();
      vPortFree(gpsData_);
    }
  }
  //}}}

  /*init() configures the uart communication related to gps_hwconfig.h,
   * and returns ESP_FAIL or ESP_OK depending on success of the initialisation*/
  esp_err_t init() override;

  /*fill_data() reads the uart data, parses them and fills the gps::data struct with
   * parsed and structured values of the Gps*/
  void fill_data();

  gps::data* gpsData_;
  gps::task_context* context_;

private:
  void consume_uart_data();
  void consume_byte(char byte);
  void parse_sentence(const char* sentence);

  const char* TAG = "GpsDriver";
  uint8_t* uart_data_;
  char* sentence_;
  uint16_t bytes_read_ = 0;
  uint16_t sentence_len_ = 0;
  bool sentence_compl_ = true;
};
//}}}
