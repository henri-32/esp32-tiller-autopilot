#include "esp_err.h"
#include "driver/i2c_master.h"

class I2cDriver
{
public:
  esp_err_t init_bus();
  i2c_master_bus_handle_t get_bus_handler() const; 


private: 
  i2c_master_bus_handle_t m_bus_handle; 
  
};
