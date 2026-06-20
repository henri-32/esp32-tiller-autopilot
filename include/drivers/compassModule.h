#pragma once
#include "types/globalTypes.h"
#include <cstdint>
#include <cstdlib>
#include "driver/i2c_master.h"


class ICompassModule {
public: 

  virtual SensorSample<uint16_t> read() const = 0; 
  virtual esp_err_t init () = 0; 

};

class CompassModule: public ICompassModule {
public:
  // Contract:
  // Purpose: Read compass heading.
  // Inputs: hardware sensor state.
  // Outputs/Side-effects: returns SensorSample; no side-effects.
  //
  explicit CompassModule(i2c_master_bus_handle_t mh): master_handle_(mh) {};  


  esp_err_t init() override; 
  int16_t read_raw(); 
  void dump(); 
  SensorSample<uint16_t> read() const override;

private: 
  i2c_master_bus_handle_t master_handle_; 
  i2c_master_dev_handle_t dev_handle_; 
};
