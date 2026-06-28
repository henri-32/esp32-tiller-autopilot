#pragma once
#include "esp_err.h"

struct NMEA_DataSet;

class IGpsDriver
{
public:
  virtual esp_err_t init() = 0;

  virtual NMEA_DataSet read_raw() const = 0;
};

class GpsDriver : public IGpsDriver
{
public:
  explicit GpsDriver() = default;

  esp_err_t init() override;

  NMEA_DataSet read_raw() const override;

private:
};

struct NMEA_DataSet
{
};
