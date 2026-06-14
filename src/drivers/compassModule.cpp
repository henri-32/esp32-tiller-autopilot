#include "drivers/compassModule.h"
#include "esp_err.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <sys/types.h>

SensorSample<uint16_t> CompassModule::read() const
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

esp_err_t CompassModule::init()
{
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = 0x0D,
      .scl_speed_hz = 10000,
  };
  i2c_master_dev_handle_t dev_handle; 
  esp_err_t device_init = i2c_master_bus_add_device(master_handle_, &dev_cfg, &dev_handle); 
  

  // TODO Maybe future checking of values for validation after init
  return device_init; 
}
