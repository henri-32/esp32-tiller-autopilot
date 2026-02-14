#include "core/config.h"
#include "core/steering/csc/deadband.h"
#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/steeringGuard.h"

#include <cstdint>
#include <unity.h>

namespace {

SteeringControllerConfig makeConfig() { return SteeringControllerConfig{}; }

void addSamples(ObservationBuffer &buffer, int16_t value, int count) {
  for (int i = 0; i < count; i++) {
    buffer.update(value);
  }
}

} // namespace

void setUp() {}
void tearDown() {}

// HeadingErrorCalculator tests
void test_heading_error_is_zero_when_current_equals_target() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.getCurrentError(300, 300);

  // Then
  TEST_ASSERT_EQUAL_INT16(0, error);
}

void test_heading_error_positive_when_current_is_left_of_target() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.getCurrentError(299, 300);

  // Then
  TEST_ASSERT_TRUE(error > 0);
}

void test_heading_error_negative_when_current_is_right_of_target() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.getCurrentError(300, 299);

  // Then
  TEST_ASSERT_TRUE(error < 0);
}

void test_heading_error_wraps_correctly_over_360_clockwise() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.getCurrentError(359, 1);

  // Then
  TEST_ASSERT_TRUE(error > 0);
}

void test_heading_error_wraps_correctly_over_360_counterclockwise() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.getCurrentError(1, 359);

  // Then
  TEST_ASSERT_TRUE(error < 0);
}

void test_heading_error_returns_shortest_rotation_direction() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.getCurrentError(350, 10);

  // Then
  TEST_ASSERT_EQUAL_INT16(20, error);
}

// Deadband tests
void test_no_action_when_error_within_deadband_positive() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.steeringTolerance_deg = 10;
  Deadband deadband(config);

  // When / Then
  TEST_ASSERT_FALSE(deadband.errorSignificant(9));
}

void test_no_action_when_error_within_deadband_negative() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.steeringTolerance_deg = 10;
  Deadband deadband(config);

  // When / Then
  TEST_ASSERT_FALSE(deadband.errorSignificant(-9));
}

void test_error_exactly_on_deadband_boundary_is_treated_as_non_significant() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.steeringTolerance_deg = 10;
  Deadband deadband(config);

  // When / Then
  TEST_ASSERT_FALSE(deadband.errorSignificant(10));
}

void test_error_just_outside_deadband_is_treated_as_significant() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.steeringTolerance_deg = 10;
  Deadband deadband(config);

  // When / Then
  TEST_ASSERT_TRUE(deadband.errorSignificant(11));
}

// SteeringGuard sample-size gating tests
void test_buffer_is_not_ready_when_empty() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.minimumSampleSize = 10;
  SteeringGuard guard(config);

  const uint32_t time = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t sampleSize = 0;

  // When / Then
  TEST_ASSERT_TRUE(guard.intentBlocked(time, lastIntent, sampleSize));
}

void test_buffer_is_not_ready_when_below_minimum_samples() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.minimumSampleSize = 10;
  SteeringGuard guard(config);

  const uint32_t time = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t sampleSize = 9;

  // When / Then
  TEST_ASSERT_TRUE(guard.intentBlocked(time, lastIntent, sampleSize));
}

void test_buffer_is_ready_when_minimum_samples_reached() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.minimumSampleSize = 10;
  SteeringGuard guard(config);

  const uint32_t time = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t sampleSize = 10;

  // When / Then
  TEST_ASSERT_FALSE(guard.intentBlocked(time, lastIntent, sampleSize));
}

// ObservationBuffer tests
void test_buffer_reset_clears_all_stored_samples() {
  // Given
  SteeringControllerConfig config = makeConfig();
  ObservationBuffer buffer(config);
  addSamples(buffer, 20, 10);

  // When
  TEST_ASSERT_TRUE(buffer.getSampleSize() != 0);
  buffer.reset();

  // Then
  TEST_ASSERT_EQUAL_UINT8(0, buffer.getSampleSize());
}

void test_median_returns_positive_when_majority_positive() {
  // Given
  SteeringControllerConfig config = makeConfig();
  ObservationBuffer buffer(config);

  const int positives = config.regulations.observationBufferSize / 2;
  const int negatives = config.regulations.observationBufferSize / 4;
  addSamples(buffer, 20, positives);
  addSamples(buffer, -20, negatives);

  // Then
  TEST_ASSERT_TRUE(buffer.getMedian() > 0);
}

void test_median_returns_negative_when_majority_negative() {
  // Given
  SteeringControllerConfig config = makeConfig();
  ObservationBuffer buffer(config);

  const int negatives = config.regulations.observationBufferSize / 2;
  const int positives = config.regulations.observationBufferSize / 4;
  addSamples(buffer, -20, negatives);
  addSamples(buffer, 20, positives);

  // Then
  TEST_ASSERT_TRUE(buffer.getMedian() < 0);
}

void test_median_returns_positive_when_one_sample_more_than_half_positive() {
  // Given
  SteeringControllerConfig config = makeConfig();
  ObservationBuffer buffer(config);

  const int negatives = (config.regulations.observationBufferSize / 2) - 1;
  const int positives = (config.regulations.observationBufferSize / 2) + 1;
  addSamples(buffer, -20, negatives);
  addSamples(buffer, 20, positives);

  // Then
  TEST_ASSERT_TRUE(buffer.getMedian() > 0);
}

// SteeringGuard cooldown tests
void test_no_second_action_within_cooldown_period() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.minimumSampleSize = 10;
  config.regulations.steeringCooldown_ms = 2000;
  SteeringGuard guard(config);

  const uint32_t lastIntent = 10000;
  const uint8_t enoughSamples = 10;

  // When / Then
  TEST_ASSERT_TRUE(guard.intentBlocked(lastIntent + 1000, lastIntent,
                                       enoughSamples));
}

void test_action_allowed_after_cooldown_expires() {
  // Given
  SteeringControllerConfig config = makeConfig();
  config.regulations.minimumSampleSize = 10;
  config.regulations.steeringCooldown_ms = 2000;
  SteeringGuard guard(config);

  const uint32_t lastIntent = 10000;
  const uint8_t enoughSamples = 10;

  // When / Then
  TEST_ASSERT_FALSE(guard.intentBlocked(lastIntent +
                                            config.regulations
                                                .steeringCooldown_ms +
                                            1,
                                        lastIntent, enoughSamples));
}

void test_no_action_when_error_oscillates_symmetrically() {
  // Given
  SteeringControllerConfig config = makeConfig();
  ObservationBuffer buffer(config);

  const int pairs = config.regulations.observationBufferSize / 2;
  for (int i = 0; i < pairs; i++) {
    buffer.update(-20);
    buffer.update(20);
  }

  // Then
  TEST_ASSERT_EQUAL_INT16(0, buffer.getMedian());
}

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
  RUN_TEST(test_median_returns_negative_when_majority_negative);
  RUN_TEST(
      test_median_returns_positive_when_one_sample_more_than_half_positive);
  RUN_TEST(test_no_second_action_within_cooldown_period);
  RUN_TEST(test_action_allowed_after_cooldown_expires);
  RUN_TEST(test_no_action_when_error_oscillates_symmetrically);

  return UNITY_END();
}
