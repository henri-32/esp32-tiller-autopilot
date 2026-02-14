#include "core/steering/sourceModels/gpsModel.h"

GPSModel::GPSModel() = default;

uint16_t GPSModel::applyFilter(uint16_t target) {
  return target;
}
