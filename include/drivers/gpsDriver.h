#pragma once
#include "config/gps_hwconfig.h"
#include "core/freertosTypes.h"
#include "core/queueServer.h"
#include "drivers/gpsTypes.h"
#include "drivers/uartDriver.h"
#include "esp_err.h"
#include "protocol/internalMessageProtocol.h"
#include <new>

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
  explicit GpsDriver(const QueueServer* const qServer, UartDriver* const uartDriver)
      //{{{
      : id_{ModuleID::GPS}, qServer_(qServer), uartDriver_(uartDriver), runtimeLogMessage_{id_}
  {
    uart_data_ = static_cast<uint8_t*>(pvPortMalloc(1024));
    sentence_ = static_cast<char*>(pvPortMalloc(GpsConfig::max_sentence_len));
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
  }
  //}}}

  esp_err_t init() override;

  void fill_payload();

  QueueBundle_t queueBundle_;
  gpsDriverData_t module_payload_;
  ModuleID id_;

private:
  void consume_uart_data();
  void consume_byte(char byte);
  void consume_sentence(const char* sentence);

  const QueueServer* qServer_;
  const UartDriver* uartDriver_;
  QueueHandle_t nmea_handle_;
  RuntimeLogMessage runtimeLogMessage_;
  uint8_t* uart_data_;
  char* sentence_;
  ssize_t bytes_read_ = 0;
  uint16_t sentence_len_ = 0;
  bool sentence_compl_ = true;
};
//}}}
