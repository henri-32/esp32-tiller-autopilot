#pragma once
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "types/sensorTypes.h"
#include <cstdint>
#include <cstdlib>

struct raw_compass_val_t;

class ICompassDriver
{
public:
  virtual raw_compass_val_t read_raw() = 0;
  virtual esp_err_t init() = 0;
};

class CompassDriver : public ICompassDriver
{
public:
  explicit CompassDriver(i2c_master_bus_handle_t mh) : master_handle_(mh) {};

  esp_err_t init() override;

  raw_compass_val_t read_raw() override;

  bool data_is_rdy();

private:
  const char* TAG = "CompassDriver::init()";
  i2c_master_bus_handle_t master_handle_;
  i2c_master_dev_handle_t dev_handle_;
};

struct raw_compass_val_t
{
  int16_t x = 0;
  int16_t y = 0;
  int16_t z = 0;
};
