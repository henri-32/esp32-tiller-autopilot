#include "controlPanel.h"
#include <unity.h>

void test_example() { TEST_ASSERT_EQUAL(1, 1); }

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_example);
  return UNITY_END();
}

ControlPanel::Intent ControlPanel::readIntent() const {
  // TODO Inputs lesen und Intent befüllen
  return {};
}
