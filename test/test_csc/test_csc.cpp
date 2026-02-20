#include "core/config.h"
#include "core/steering/csc/csc.h"
#include "core/steering/csc/deadband.h"
#include "core/steering/csc/headingErrorCalculator.h"
#include "core/steering/csc/observationBuffer.h"
#include "core/steering/csc/steeringGuard.h"

#include <cstdint>
#include <unity.h>

namespace {

SteeringRegulationConfig makeRegConfig() { return SteeringRegulationConfig{}; }

void addSamples(ObservationBuffer &buffer, int16_t value, int count) {
  for (int i = 0; i < count; i++) {
    buffer.update(value, static_cast<uint32_t>(i));
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
  const int16_t error = calc.calculateError(300, 300);

  // Then
  TEST_ASSERT_EQUAL_INT16(0, error);
}

void test_heading_error_positive_when_current_is_left_of_target() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.calculateError(299, 300);

  // Then
  TEST_ASSERT_TRUE(error > 0);
}

void test_heading_error_negative_when_current_is_right_of_target() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.calculateError(300, 299);

  // Then
  TEST_ASSERT_TRUE(error < 0);
}

void test_heading_error_wraps_correctly_over_360_clockwise() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.calculateError(359, 1);

  // Then
  TEST_ASSERT_TRUE(error > 0);
}

void test_heading_error_wraps_correctly_over_360_counterclockwise() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.calculateError(1, 359);

  // Then
  TEST_ASSERT_TRUE(error < 0);
}

void test_heading_error_returns_shortest_rotation_direction() {
  // Given
  HeadingErrorCalculator calc;

  // When
  const int16_t error = calc.calculateError(350, 10);

  // Then
  TEST_ASSERT_EQUAL_INT16(20, error);
}

// Deadband tests
void test_no_action_when_error_within_deadband_positive() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 10;
  Deadband deadband(config);

  // When / Then
  TEST_ASSERT_FALSE(deadband.errorSignificant(9));
}

void test_no_action_when_error_within_deadband_negative() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 10;
  Deadband deadband(config);

  // When / Then
  TEST_ASSERT_FALSE(deadband.errorSignificant(-9));
}

void test_error_exactly_on_deadband_boundary_is_treated_as_non_significant() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 10;
  Deadband deadband(config);

  // When / Then
  TEST_ASSERT_FALSE(deadband.errorSignificant(10));
}

void test_error_just_outside_deadband_is_treated_as_significant() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 10;
  Deadband deadband(config);

  // When / Then
  TEST_ASSERT_TRUE(deadband.errorSignificant(11));
}

// SteeringGuard sample-size gating tests
void test_buffer_is_not_ready_when_empty() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.minimumSampleSize = 10;
  SteeringGuard guard(config);

  const uint32_t time = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t sampleSize = 0;
  const int16_t median = 0;
  const float omega = 0;

  // When / Then
  TEST_ASSERT_TRUE(
      guard.intentBlocked(time, lastIntent, sampleSize, median, omega));
}

void test_buffer_is_not_ready_when_below_minimum_samples() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.minimumSampleSize = 10;
  SteeringGuard guard(config);

  const uint32_t time = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t sampleSize = 9;
  const int16_t median = 0;
  const float omega = 0;

  // When / Then
  TEST_ASSERT_TRUE(
      guard.intentBlocked(time, lastIntent, sampleSize, median, omega));
}

void test_buffer_is_ready_when_minimum_samples_reached() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.minimumSampleSize = 10;
  SteeringGuard guard(config);

  const uint32_t time = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t sampleSize = 10;
  const int16_t median = config.steeringTolerance_deg + 1;
  const float omega = 0;
  // When / Then
  TEST_ASSERT_FALSE(
      guard.intentBlocked(time, lastIntent, sampleSize, median, omega));
}

// ObservationBuffer tests
void test_buffer_reset_clears_all_stored_samples() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
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
  SteeringRegulationConfig config = makeRegConfig();
  ObservationBuffer buffer(config);

  const int positives = config.observationBufferSize / 2;
  const int negatives = config.observationBufferSize / 4;
  addSamples(buffer, 20, positives);
  addSamples(buffer, -20, negatives);

  // Then
  TEST_ASSERT_TRUE(buffer.getMedian() > 0);
}

void test_median_returns_negative_when_majority_negative() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  ObservationBuffer buffer(config);

  const int negatives = config.observationBufferSize / 2;
  const int positives = config.observationBufferSize / 4;
  addSamples(buffer, -20, negatives);
  addSamples(buffer, 20, positives);

  // Then
  TEST_ASSERT_TRUE(buffer.getMedian() < 0);
}

void test_median_returns_positive_when_majority_samples_are_positive() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  ObservationBuffer buffer(config);

  // Keep total samples below capacity to avoid buffer reset-on-full behavior.
  const int negatives = (config.observationBufferSize / 2) - 1;
  const int positives = (config.observationBufferSize / 2);
  for (int i = 0; i < negatives; ++i) {
    buffer.update(-20, static_cast<uint32_t>(i));
  }
  for (int i = 0; i < positives; ++i) {
    buffer.update(20, static_cast<uint32_t>(negatives + i));
  }

  // Then
  TEST_ASSERT_TRUE(buffer.getMedian() > 0);
}

void test_omega_is_positive_when_error_decreases_over_time() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.omegaRobust = 3;
  ObservationBuffer buffer(config);

  // Error drops from +20 to +10 in 1 second => -d(error)/dt = +10 deg/s
  buffer.update(20, 1000);
  buffer.update(10, 2000);

  // When
  const float omega = buffer.getOmega(2000);

  // Then
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, omega);
}

void test_omega_is_negative_when_error_increases_over_time() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.omegaRobust = 3;
  ObservationBuffer buffer(config);

  // Error rises from +10 to +20 in 1 second => -d(error)/dt = -10 deg/s
  buffer.update(10, 1000);
  buffer.update(20, 2000);

  // When
  const float omega = buffer.getOmega(2000);

  // Then
  TEST_ASSERT_FLOAT_WITHIN(0.001f, -10.0f, omega);
}

void test_omega_uses_only_configured_robust_window() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.omegaRobust = 3;
  ObservationBuffer buffer(config);

  // Last 3 samples are used (80@1000 -> 40@3000), first sample ignored.
  buffer.update(100, 0);
  buffer.update(80, 1000);
  buffer.update(60, 2000);
  buffer.update(40, 3000);

  // When
  const float omega = buffer.getOmega(3000);

  // Then
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 20.0f, omega);
}

void test_omega_returns_zero_when_window_timestamps_are_not_increasing() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.omegaRobust = 3;
  ObservationBuffer buffer(config);

  buffer.update(20, 1000);
  buffer.update(10, 1000);

  // When
  const float omega = buffer.getOmega(1000);

  // Then
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, omega);
}

// SteeringGuard cooldown tests
void test_no_second_action_within_cooldown_period() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.minimumSampleSize = 10;
  config.steeringCooldown_ms = 2000;
  SteeringGuard guard(config);

  const uint32_t lastIntent = 10000;
  const uint8_t enoughSamples = 10;
  const int16_t median = 0;
  const float omega = 0;

  // When / Then
  TEST_ASSERT_TRUE(guard.intentBlocked(lastIntent + 1000, lastIntent,
                                       enoughSamples, median, omega));
}

void test_no_action_within_omegaDeadband() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  SteeringGuard guard(config);

  const uint32_t lastIntent = 10000;
  const uint8_t enoughSamples = 10;
  const int16_t median = 0;
  const float omega = 0;

  TEST_ASSERT_TRUE(guard.intentBlocked(lastIntent + 1000, lastIntent,
                                       enoughSamples, median, omega));
};

void test_omega_guard_blocks_positive_median_when_omega_exceeds_deadband() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringCooldown_ms = 0;
  config.minimumSampleSize = 5;
  config.steeringTolerance_deg = 5;
  config.omegaDeadband = 0.2f;
  SteeringGuard guard(config);

  const uint32_t loopTimestamp = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t enoughSamples = 5;
  const int16_t median = config.steeringTolerance_deg + 1;
  const float omega = config.omegaDeadband + 0.05f;

  // When / Then
  TEST_ASSERT_TRUE(guard.intentBlocked(loopTimestamp, lastIntent, enoughSamples,
                                       median, omega));
}

void test_omega_guard_blocks_negative_median_when_omega_exceeds_deadband() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringCooldown_ms = 0;
  config.minimumSampleSize = 5;
  config.steeringTolerance_deg = 5;
  config.omegaDeadband = 0.2f;
  SteeringGuard guard(config);

  const uint32_t loopTimestamp = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t enoughSamples = 5;
  const int16_t median = -(config.steeringTolerance_deg + 1);
  const float omega = -(config.omegaDeadband + 0.05f);

  // When / Then
  TEST_ASSERT_TRUE(guard.intentBlocked(loopTimestamp, lastIntent, enoughSamples,
                                       median, omega));
}

void test_omega_guard_allows_action_when_omega_within_deadband() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringCooldown_ms = 0;
  config.minimumSampleSize = 5;
  config.steeringTolerance_deg = 5;
  config.omegaDeadband = 0.2f;
  SteeringGuard guard(config);

  const uint32_t loopTimestamp = 10000;
  const uint32_t lastIntent = 0;
  const uint8_t enoughSamples = 5;
  const int16_t median = config.steeringTolerance_deg + 1;
  const float omega = config.omegaDeadband - 0.05f;

  // When / Then
  TEST_ASSERT_FALSE(guard.intentBlocked(loopTimestamp, lastIntent,
                                        enoughSamples, median, omega));
}

void test_action_allowed_after_cooldown_expires() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  config.minimumSampleSize = 10;
  config.steeringCooldown_ms = 2000;
  SteeringGuard guard(config);

  const uint32_t lastIntent = 10000;
  const uint8_t enoughSamples = 10;
  const int16_t median = config.steeringTolerance_deg + 1;
  const float omega = 0;

  // When / Then
  TEST_ASSERT_FALSE(
      guard.intentBlocked(lastIntent + config.steeringCooldown_ms + 1,
                          lastIntent, enoughSamples, median, omega));
}

void test_no_action_when_error_oscillates_symmetrically() {
  // Given
  SteeringRegulationConfig config = makeRegConfig();
  ObservationBuffer buffer(config);

  const int pairs = config.observationBufferSize / 2;
  for (int i = 0; i < pairs; i++) {
    buffer.update(-20, static_cast<uint32_t>(2 * i));
    buffer.update(20, static_cast<uint32_t>(2 * i + 1));
  }

  // Then
  TEST_ASSERT_EQUAL_INT16(0, buffer.getMedian());
}
void test_csc_emits_intent_after_minimum_samples_reached() {

  SteeringRegulationConfig config;
  config.steeringTolerance_deg = 5;
  config.minimumSampleSize = 5;
  config.minimumTimeBtwObs_ms = 0;
  config.pauseForValidObsAfterImpulse_ms = 0;
  config.steeringCooldown_ms = 0;

  CoreSteeringController csc(config);

  csc.setInternalTarget(100);

  std::optional<SteeringIntent> intent;

  uint32_t time = 0;

  for (int i = 0; i < 10; ++i) {

    csc.currentHDG(80); // +20° Fehler → Right
    intent = csc.tick(time++);

    if (intent.has_value())
      break;
  }

  TEST_ASSERT_TRUE(intent.has_value());
  TEST_ASSERT_EQUAL(SteeringDirection::Right, intent->dir);
}

void test_csc_does_not_emit_intent_inside_deadband() {

  SteeringRegulationConfig config;
  config.steeringTolerance_deg = 10;
  config.minimumSampleSize = 5;
  config.minimumTimeBtwObs_ms = 0;
  config.pauseForValidObsAfterImpulse_ms = 0;
  config.steeringCooldown_ms = 0;

  CoreSteeringController csc(config);

  csc.setInternalTarget(100);

  std::optional<SteeringIntent> intent;

  for (int i = 0; i < 20; ++i) {
    csc.currentHDG(95); // +5° → innerhalb Toleranz
    intent = csc.tick(i);
  }

  TEST_ASSERT_FALSE(intent.has_value());
}

void test_csc_resets_observation_after_intent() {

  SteeringRegulationConfig config;
  config.steeringTolerance_deg = 5;
  config.minimumSampleSize = 3;
  config.minimumTimeBtwObs_ms = 0;
  config.pauseForValidObsAfterImpulse_ms = 0;
  config.steeringCooldown_ms = 0;

  CoreSteeringController csc(config);

  csc.setInternalTarget(100);

  uint32_t time = 0;
  bool firstIntentOccurred = false;
  bool secondIntentOccurred = false;

  for (int i = 0; i < 20; ++i) {

    csc.currentHDG(80);
    auto intent = csc.tick(time++);

    if (intent && !firstIntentOccurred) {
      firstIntentOccurred = true;
    } else if (intent && firstIntentOccurred) {
      secondIntentOccurred = true;
      break;
    }
  }

  TEST_ASSERT_TRUE(firstIntentOccurred);
  TEST_ASSERT_TRUE(secondIntentOccurred);
}

void test_csc_respects_time_gates_and_cooldown() {

  // ---------- Config ----------
  SteeringRegulationConfig config;
  config.steeringTolerance_deg = 5;
  config.minimumSampleSize = 5;

  config.minimumTimeBtwObs_ms = 1000;
  config.pauseForValidObsAfterImpulse_ms = 2000;
  config.steeringCooldown_ms = 3000;

  CoreSteeringController csc(config);

  csc.setInternalTarget(100);

  uint32_t time = 0;

  bool firstIntent = false;
  bool secondIntent = false;

  // ---------- Simulation ----------
  for (int i = 0; i < 30; ++i) {

    csc.currentHDG(80); // +20° Fehler

    auto intent = csc.tick(time);

    if (intent && !firstIntent) {
      firstIntent = true;
    } else if (intent && firstIntent) {
      secondIntent = true;
      break;
    }

    time += 1000; // 1 Hz loop
  }

  TEST_ASSERT_TRUE(firstIntent);
  TEST_ASSERT_TRUE(secondIntent);
}

void test_error_just_outside_deadband_negative_is_treated_as_significant() {
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 10;
  Deadband deadband(config);

  TEST_ASSERT_TRUE(deadband.errorSignificant(-11));
}

void test_counter_does_not_fire_without_prior_intent_value() {
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 1;
  config.counterNearTargetWindow_deg = 2;
  config.counterOmegaMinSampleSize = 2;
  config.omegaThresholdForCounter = 0.01f;
  config.minimumSampleSize = 2;
  config.minimumTimeBtwObs_ms = 0;
  config.pauseForValidObsAfterImpulse_ms = 0;
  config.steeringCooldown_ms = 5000;
  config.counterCooldown_ms = 0;
  config.counterTimerGuard_ms = 0;

  CoreSteeringController csc(config);
  csc.setInternalTarget(100);

  csc.currentHDG(98); // error +2
  auto i1 = csc.tick(1000);
  csc.currentHDG(99); // error +1 -> positive omega
  auto i2 = csc.tick(2000);

  TEST_ASSERT_FALSE(i1.has_value());
  TEST_ASSERT_FALSE(i2.has_value());
}

void test_counter_fires_and_maps_impulse_from_positive_omega() {
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 1;
  config.counterNearTargetWindow_deg = 2;
  config.counterOmegaMinSampleSize = 2;
  config.omegaThresholdForCounter = 0.01f;
  config.minimumSampleSize = 1;
  config.minimumTimeBtwObs_ms = 0;
  config.pauseForValidObsAfterImpulse_ms = 0;
  config.steeringCooldown_ms = 5000;
  config.counterCooldown_ms = 0;
  config.counterTimerGuard_ms = 0;

  CoreSteeringController csc(config);
  csc.setInternalTarget(100);

  // Seed regular intent (+error => Right), needed for counter direction basis.
  csc.currentHDG(80);
  auto seedIntent = csc.tick(6000);
  TEST_ASSERT_TRUE(seedIntent.has_value());
  TEST_ASSERT_EQUAL(SteeringDirection::Right, seedIntent->dir);

  csc.currentHDG(98); // error +2
  auto prep = csc.tick(7000);
  csc.currentHDG(99); // error +1 over 1s => omega +1 => impulse 60
  auto counter = csc.tick(8000);

  TEST_ASSERT_FALSE(prep.has_value());
  TEST_ASSERT_TRUE(counter.has_value());
  TEST_ASSERT_EQUAL(SteeringDirection::Left, counter->dir);
  TEST_ASSERT_EQUAL_UINT8(60, counter->abstractImpulse_0_100);
}

void test_counter_is_blocked_by_counter_cooldown() {
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 1;
  config.counterNearTargetWindow_deg = 2;
  config.counterOmegaMinSampleSize = 2;
  config.omegaThresholdForCounter = 0.01f;
  config.minimumSampleSize = 1;
  config.minimumTimeBtwObs_ms = 0;
  config.pauseForValidObsAfterImpulse_ms = 0;
  config.steeringCooldown_ms = 5000;
  config.counterCooldown_ms = 5000;
  config.counterTimerGuard_ms = 0;

  CoreSteeringController csc(config);
  csc.setInternalTarget(100);

  csc.currentHDG(80);
  auto seedIntent = csc.tick(6000);
  TEST_ASSERT_TRUE(seedIntent.has_value());

  csc.currentHDG(98);
  (void)csc.tick(7000);
  csc.currentHDG(99);
  auto firstCounter = csc.tick(8000);
  TEST_ASSERT_TRUE(firstCounter.has_value());

  // Build counter conditions again, but still inside counter cooldown.
  csc.currentHDG(98);
  (void)csc.tick(9000);
  csc.currentHDG(99);
  auto blockedCounter = csc.tick(10000);

  TEST_ASSERT_FALSE(blockedCounter.has_value());
}

void test_counter_is_blocked_when_abs_omega_below_counter_threshold() {
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 1;
  config.counterNearTargetWindow_deg = 2;
  config.counterOmegaMinSampleSize = 2;
  config.omegaThresholdForCounter = 2.0f; // higher than produced omega (+1)
  config.minimumSampleSize = 1;
  config.minimumTimeBtwObs_ms = 0;
  config.pauseForValidObsAfterImpulse_ms = 0;
  config.steeringCooldown_ms = 5000;
  config.counterCooldown_ms = 0;
  config.counterTimerGuard_ms = 0;

  CoreSteeringController csc(config);
  csc.setInternalTarget(100);

  csc.currentHDG(80);
  auto seedIntent = csc.tick(6000);
  TEST_ASSERT_TRUE(seedIntent.has_value());

  csc.currentHDG(98);
  (void)csc.tick(7000);
  csc.currentHDG(99);
  auto counter = csc.tick(8000);

  TEST_ASSERT_FALSE(counter.has_value());
}

void test_counter_fires_on_negative_side_with_negative_omega() {
  SteeringRegulationConfig config = makeRegConfig();
  config.steeringTolerance_deg = 1;
  config.counterNearTargetWindow_deg = 2;
  config.counterOmegaMinSampleSize = 2;
  config.omegaThresholdForCounter = 0.01f;
  config.minimumSampleSize = 1;
  config.minimumTimeBtwObs_ms = 0;
  config.pauseForValidObsAfterImpulse_ms = 0;
  config.steeringCooldown_ms = 5000;
  config.counterCooldown_ms = 0;
  config.counterTimerGuard_ms = 0;

  CoreSteeringController csc(config);
  csc.setInternalTarget(100);

  // Seed regular intent with negative error => Left
  csc.currentHDG(120);
  auto seedIntent = csc.tick(6000);
  TEST_ASSERT_TRUE(seedIntent.has_value());
  TEST_ASSERT_EQUAL(SteeringDirection::Left, seedIntent->dir);

  csc.currentHDG(102); // error -2
  (void)csc.tick(7000);
  csc.currentHDG(101); // error -1 over 1s => omega -1 => impulse 60
  auto counter = csc.tick(8000);

  TEST_ASSERT_TRUE(counter.has_value());
  TEST_ASSERT_EQUAL(SteeringDirection::Right, counter->dir);
  TEST_ASSERT_EQUAL_UINT8(60, counter->abstractImpulse_0_100);
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
  RUN_TEST(test_median_returns_positive_when_majority_samples_are_positive);
  RUN_TEST(test_omega_is_positive_when_error_decreases_over_time);
  RUN_TEST(test_omega_is_negative_when_error_increases_over_time);
  RUN_TEST(test_omega_uses_only_configured_robust_window);
  RUN_TEST(test_omega_returns_zero_when_window_timestamps_are_not_increasing);
  RUN_TEST(test_no_second_action_within_cooldown_period);
  RUN_TEST(test_action_allowed_after_cooldown_expires);
  RUN_TEST(test_no_action_when_error_oscillates_symmetrically);
  RUN_TEST(test_csc_emits_intent_after_minimum_samples_reached);
  /*RUN_TEST(test_csc_does_not_emit_intent_inside_deadband); Deadband aktuell
   * deaktiviert*/
  RUN_TEST(test_csc_resets_observation_after_intent);
  RUN_TEST(test_csc_respects_time_gates_and_cooldown);
  RUN_TEST(test_error_just_outside_deadband_negative_is_treated_as_significant);
  RUN_TEST(test_no_action_within_omegaDeadband);
  RUN_TEST(test_omega_guard_blocks_positive_median_when_omega_exceeds_deadband);
  RUN_TEST(test_omega_guard_blocks_negative_median_when_omega_exceeds_deadband);
  RUN_TEST(test_omega_guard_allows_action_when_omega_within_deadband);
  RUN_TEST(test_counter_does_not_fire_without_prior_intent_value);
  RUN_TEST(test_counter_fires_and_maps_impulse_from_positive_omega);
  RUN_TEST(test_counter_is_blocked_by_counter_cooldown);
  RUN_TEST(test_counter_is_blocked_when_abs_omega_below_counter_threshold);
  RUN_TEST(test_counter_fires_on_negative_side_with_negative_omega);

  return UNITY_END();
}
