#include "driver/i2c_master.h"
#include "esp_err.h"

class I2cDriver
{
public:
  esp_err_t init();
  i2c_master_bus_handle_t get_master_bus_handle() const;

private:
  const char* TAG = "I2C_DRIVER";
  i2c_master_bus_handle_t bus_handle_{};
};
