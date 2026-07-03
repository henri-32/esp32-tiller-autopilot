#pragma once
#include "esp_err.h"
#include "config/gps_hwconfig.h"

struct NMEA_DataSet;

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
  NMEA_DataSet parse_msg();
};

struct NMEA_DataSet
{
  int test = 0; 
};

class full_sentence
{
public:
  full_sentence() = default;
  ~full_sentence() {free(sentence_);};

  const char* get_sentence();

private:
  void read_buffer();
  void consume_uart_data();
  void consume_byte(char byte);

  uint8_t uart_data_[GpsConfig::RX_buffer + 1];
  uint16_t bytes_read_; 
  uint16_t sentence_len_ = 0;
  bool sentence_compl_ = true;
  char* sentence_ =(char*)malloc(2000);
  const char* last_compl_sentence_ = {};


};
