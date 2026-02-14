#include "core/config.h"
#include "core/steering/csc/csc.h"

#include <cstdint>
#include <unity.h>

void setUp() {}
void tearDown() {}

void test_heading_error_is_zero_when_current_equals_target() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  int16_t error = csc.m_errorCalculator.getCurrentError(300, 300);

  TEST_ASSERT_TRUE(error == 0);
}

void test_heading_error_positive_when_current_is_left_of_target() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  int16_t error = csc.m_errorCalculator.getCurrentError(299, 300);

  TEST_ASSERT_TRUE(error > 0);
}

void test_heading_error_negative_when_current_is_right_of_target() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  int16_t error = csc.m_errorCalculator.getCurrentError(300, 299);

  TEST_ASSERT_TRUE(error < 0);
}

void test_heading_error_wraps_correctly_over_360_clockwise() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  int error = csc.m_errorCalculator.getCurrentError(359, 1);

  TEST_ASSERT_TRUE(error > 0);
}
void test_heading_error_wraps_correctly_over_360_counterclockwise() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  int16_t error = csc.m_errorCalculator.getCurrentError(1, 359);

  TEST_ASSERT_TRUE(error < 0);
}

void test_heading_error_returns_shortest_rotation_direction() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  int16_t error = csc.m_errorCalculator.getCurrentError(350, 10);

  TEST_ASSERT_TRUE(error == 20);
}

void test_no_action_when_error_within_deadband_positive() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  config.regulations.steeringTolerance_deg = 10;

  TEST_ASSERT_FALSE(csc.m_deadband.errorSignificant(9));
}

void test_no_action_when_error_within_deadband_negative() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  config.regulations.steeringTolerance_deg = 10;

  TEST_ASSERT_FALSE(csc.m_deadband.errorSignificant(-9));
}

void test_error_exactly_on_deadband_boundary_is_treated_as_non_significant() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  config.regulations.steeringTolerance_deg = 10;

  TEST_ASSERT_TRUE(!csc.m_deadband.errorSignificant(10));
}

void test_error_just_outside_deadband_is_treated_as_significant() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  config.regulations.steeringTolerance_deg = 10;

  TEST_ASSERT_TRUE(csc.m_deadband.errorSignificant(11));
}

void test_buffer_is_not_ready_when_empty() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  config.regulations.minimumSampleSize = 10;
  uint32_t time = 10000;
  uint32_t lastIntent = 0;
  uint8_t sampleSize = 0;

  TEST_ASSERT_TRUE(
      csc.m_steeringGuard.activeIntentGuard(time, lastIntent, sampleSize));
}

void test_buffer_is_not_ready_when_below_minimum_samples() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  config.regulations.minimumSampleSize = 10;
  uint32_t time = 10000;
  uint32_t lastIntent = 0;
  uint8_t sampleSize = 9;

  TEST_ASSERT_TRUE(
      csc.m_steeringGuard.activeIntentGuard(time, lastIntent, sampleSize));
}
void test_buffer_is_ready_when_minimum_samples_reached() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  config.regulations.minimumSampleSize = 10;
  uint32_t time = 10000;
  uint32_t lastIntent = 0;
  uint8_t sampleSize = 10;

  TEST_ASSERT_TRUE(
      !csc.m_steeringGuard.activeIntentGuard(time, lastIntent, sampleSize));
}
void test_buffer_reset_clears_all_stored_samples() {
  SteeringController_Config config;
  CoreSteeringController csc(config);
  uint8_t median = 0;
  uint32_t time = 0;

  config.regulations.minimumSampleSize = 10;
  csc.setInternalTarget(300);

  // Weniger als minimal Sample Size, damit nicht signifikant und reset
  for (int i = 0; i < 10; i++) {
    time += 1000;
    csc.currentHDG(200); // Konstanter Fehler
    csc.tick(time);
  };
  /*Array wurde beschrieben */
  TEST_ASSERT_TRUE(csc.m_observationBuffer.getSampleSize() != 0);
  csc.m_observationBuffer.reset();

  /*Trotzdem ist das Array am Ende leer*/
  TEST_ASSERT_TRUE(csc.m_observationBuffer.getSampleSize() == 0);
}

void test_median_returns_positive_when_majority_positive() {
  SteeringController_Config config;
  CoreSteeringController csc(config);
  uint32_t time = 0;
  uint8_t median = 0;

  csc.setInternalTarget(300);

  /* Hälfte des Buffers positiv*/
  for (int i = 0; i < config.regulations.observationBufferSize / 2; i++) {
    time += 1000;
    csc.currentHDG(200); // Konstanter Fehler
    csc.tick(time);
  };

  /*viertel des Buffers negativ*/
  for (int i = 0; i < config.regulations.observationBufferSize / 4; i++) {
    time += 1000;
    csc.currentHDG(320);
    csc.tick(time);
  };

  TEST_ASSERT_TRUE(csc.m_observationBuffer.getMedian() > 0);
}
void test_median_returns_negative_when_majority_negative() {}
void test_median_returns_no_action_when_exactly_symmetric() {}
void test_median_returns_positive_when_one_sample_more_than_half_positive() {}
void test_median_is_not_affected_by_outliers() {}

void test_no_action_when_all_samples_within_deadband_even_if_buffer_full() {}
void test_no_action_when_only_few_significant_samples_present() {}
void test_action_occurs_when_minimum_significant_samples_reached() {}
void test_majority_direction_overrides_small_number_of_counter_samples() {}

void test_no_second_action_within_cooldown_period() {}
void test_action_allowed_after_cooldown_expires() {}
void test_cooldown_does_not_block_first_action() {}

void test_buffer_resets_after_action_is_emitted() {}
void test_new_observations_after_reset_start_fresh_evaluation() {}
void test_opposite_direction_after_reset_requires_new_evidence() {}

void test_no_action_when_error_oscillates_symmetrically() {}
void test_action_when_error_drift_is_slow_but_consistent() {}
void test_no_action_when_error_fluctuates_randomly_around_zero() {}
void test_action_when_error_remains_constant_over_time() {}

void test_system_handles_maximum_heading_values_without_overflow() {}
void test_system_handles_zero_and_360_equally() {}
void test_system_does_not_emit_action_without_target_set() {}
void test_system_does_not_emit_action_before_any_heading_received() {}

void test_no_action_when_inside_tolerance_for_full_buffer_duration() {}
void test_action_to_left_when_consistently_right_of_target() {}
void test_action_to_right_when_consistently_left_of_target() {}
void test_no_action_on_exact_symmetry_even_with_full_buffer() {}
void test_action_requires_minimum_observation_count() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_heading_error_is_zero_when_current_equals_target);
  RUN_TEST(test_heading_error_positive_when_current_is_left_of_target);
  RUN_TEST(test_heading_error_negative_when_current_is_right_of_target);
  RUN_TEST(test_heading_error_wraps_correctly_over_360_clockwise);
  RUN_TEST(test_heading_error_wraps_correctly_over_360_counterclockwise);
  RUN_TEST(test_heading_error_returns_shortest_rotation_direction);
  RUN_TEST(test_no_action_when_error_within_deadband_positive);
  RUN_TEST(test_no_action_when_error_within_deadband_negative);
  RUN_TEST(
      test_error_exactly_on_deadband_boundary_is_treated_as_non_significant);
  RUN_TEST(test_error_just_outside_deadband_is_treated_as_significant);
  RUN_TEST(test_buffer_is_not_ready_when_empty);
  RUN_TEST(test_buffer_is_not_ready_when_below_minimum_samples);
  RUN_TEST(test_buffer_is_ready_when_minimum_samples_reached);
  RUN_TEST(test_buffer_reset_clears_all_stored_samples);
  RUN_TEST(test_median_returns_positive_when_majority_positive);

  return UNITY_END();
}
