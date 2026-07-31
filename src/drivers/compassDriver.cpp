#include "drivers/compassDriver.h"
#include "config/i2c_hwconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "navigation/navigationTypes.h"
#include <cstdint>
#include <math.h>

using CCfg = CompassConfig;

esp_err_t CompassDriver::init()
//{{{
{
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = CCfg::dev_addr_length,
      .device_address = CCfg::device_adress,
      .scl_speed_hz = CCfg::scl_speed_hz,
  };

  esp_err_t device_init = i2c_master_bus_add_device(master_handle_, &dev_cfg, &dev_handle_);
  ESP_LOGI(TAG, "i2c_master_bus_add_device(): %s", esp_err_to_name(device_init));

  esp_err_t probe = i2c_master_probe(master_handle_, CCfg::device_adress, CCfg::write_timeout_ms);
  ESP_LOGI(TAG, "i2c_master_probe(): %s", esp_err_to_name(probe));

  const uint8_t data[] = {CCfg::control_reg1, CCfg::control_val};
  esp_err_t control_register_write =
      i2c_master_transmit(dev_handle_, data, sizeof(data), CCfg::write_timeout_ms);
  ESP_LOGI(TAG, "i2c_master_transmit(): %s", esp_err_to_name(control_register_write));

  if (device_init == ESP_OK && control_register_write == ESP_OK)
  {
    return ESP_OK;
  }
  else
  {
    return ESP_FAIL;
  }
  // TODO Maybe future checking of values for validation after init
}
//}}}

raw_compass_val_t CompassDriver::read_raw()
//{{{
{
  uint8_t data_reg = CCfg::compass_data_reg_start_addr;
  uint8_t data[CCfg::compass_data_reg_byte_len] = {};
  esp_err_t read = i2c_master_transmit_receive(dev_handle_, &data_reg, 1, data,
                                               CCfg::compass_data_reg_byte_len, 20);

  return {.x = static_cast<int16_t>((data[1] << 8) | data[0]),
          .y = static_cast<int16_t>((data[3] << 8) | data[2]),
          .z = static_cast<int16_t>((data[5] << 8) | data[4])};
}
//}}}

bool CompassDriver::data_is_rdy()
//{{{
{
  uint8_t data_reg = CCfg::compass_status_reg;
  uint8_t data[] = {0x0};

  esp_err_t read = i2c_master_transmit_receive(dev_handle_, &data_reg, 1, data, 1, 20);
  ESP_LOGI(TAG, "i2c_master_transmit_receive(): %s", esp_err_to_name(read));

  return (data[0] & 0b00000001) != 0;
}
//}}}
