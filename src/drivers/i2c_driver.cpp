#include "drivers/i2c_driver.h"
#include "esp_log.h"
#include "hardwareconfig.h"

esp_err_t I2cDriver::init()
{
  i2c_master_bus_config_t i2c_master_config = {};
  i2c_master_config.clk_source = I2cConfig::clk_source;
  i2c_master_config.i2c_port = I2cConfig::i2c_port;
  i2c_master_config.sda_io_num = I2cConfig::sda_io_num;
  i2c_master_config.scl_io_num = I2cConfig::scl_io_num;
  i2c_master_config.glitch_ignore_cnt = I2cConfig::glitch_ignore_cnt;
  i2c_master_config.flags.enable_internal_pullup = I2cConfig::enable_internal_pullup;

  esp_err_t master_init = i2c_new_master_bus(&i2c_master_config, &bus_handle_);
  ESP_LOGI(TAG, "i2c_new_master_bus(): %s", esp_err_to_name(master_init));
  return master_init;
}

i2c_master_bus_handle_t I2cDriver::get_master_bus_handle() const
{
  return bus_handle_;
}
