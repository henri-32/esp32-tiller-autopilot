#include "core/steering/sourceModels/windModel.h"

WindModel::WindModel() = default;

uint16_t WindModel::applyFilter(uint16_t target) {
  return target;
}
