#include "drivers/compassModule.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hardwareconfig.h"
#include "types/globalTypes.h"
#include <cstdint>

using CCfg = CompassConfig;

SensorSample<uint16_t> CompassModule::read() const
//{{{
{
  SensorSample<uint16_t> sample;

  const uint16_t raw = 0;
  if (true)
  {
    sample.value = raw;
    sample.valid = true;
  };
  return sample;
}
//}}}

esp_err_t CompassModule::init()
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
  return device_init;
}
//}}}

raw_compass_val_t CompassModule::read_raw()
//{{{
{
  uint8_t data_reg = CCfg::compassdata_register_start;
  uint8_t data[CCfg::compassdata_register_bytes] = {};
  esp_err_t read = i2c_master_transmit_receive(dev_handle_, &data_reg, 1, data,
                                               CCfg::compassdata_register_bytes, 20);

  return {.x = static_cast<int16_t>((data[1] << 8) | data[0]),
          .y = static_cast<int16_t>((data[3] << 8) | data[2]),
          .z = static_cast<int16_t>((data[5] << 8) | data[4])};
}
//}}}

void CompassModule::dump()
//{{{
{
  const char* TAG = "I2C_DUMP";
  uint8_t dumps = 0;

  while (dumps <= 10)
  {

    for (uint8_t reg = 0x00; reg <= 0x3F; ++reg)
    {
      uint8_t value = 0;
      esp_err_t err = i2c_master_transmit_receive(dev_handle_, &reg, 1, &value, 1, 100);

      if (err == ESP_OK)
      {
        ESP_LOGI(TAG, "reg 0x%02X -> 0x%02X", reg, value);
      }
      else
      {
        ESP_LOGW(TAG, "reg 0x%02X failed: %s", reg, esp_err_to_name(err));
      }
    }
    dumps++;
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
//}}}
