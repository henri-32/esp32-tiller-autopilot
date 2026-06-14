#include "drivers/i2c_driver.h"
#include "driver/gpio.h"

esp_err_t I2cDriver::init_bus()
{
  i2c_master_bus_config_t i2c_master_config = {};
  i2c_master_config.clk_source = I2C_CLK_SRC_DEFAULT;
  i2c_master_config.i2c_port = I2C_NUM_0;
  i2c_master_config.sda_io_num = GPIO_NUM_21;
  i2c_master_config.scl_io_num = GPIO_NUM_22;
  i2c_master_config.glitch_ignore_cnt = 7;
  i2c_master_config.flags.enable_internal_pullup = true;

  i2c_master_bus_handle_t bus_handle{};
  esp_err_t master_init = i2c_new_master_bus(&i2c_master_config, &bus_handle);
  return master_init;
  m_bus_handle = bus_handle;
}

i2c_master_bus_handle_t I2cDriver::get_bus_handler() const
{
  return m_bus_handle;
}
