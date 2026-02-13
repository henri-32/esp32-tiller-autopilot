#include "core/config.h"
#include "core/steering/csc.h"
#include "types/globalTypes.h"
#include <cstdint>
#include <unity.h>

void setUp() {}
void tearDown() {}

void test_no_correction_inside_tolerance() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  csc.setInternalTarget(100);

  for (int i = 0; i < config.regulations.observationBufferSize;
       i++) {            // Ein Buffer Simuliert
    csc.currentHDG(105); // +5°, default tolerance = 10°
    auto intent = csc.tick(i * 1000);
    TEST_ASSERT_FALSE(intent.has_value());
  }
}

void test_correction_steady_20dg_to_far_left() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  csc.setInternalTarget(300);
  bool intentOccurred = false;
  uint32_t time = 0;

  for (int i = 0; i < config.regulations.observationBufferSize;
       i++) // 1 Buffer simuliert
  {
    time += 1000; // 1 Hz Takt

    csc.currentHDG(280); // -20° Fehler konstant

    auto intent = csc.tick(time);

    if (intent.has_value()) {
      intentOccurred = true;

      TEST_ASSERT_EQUAL(SteeringDirection::Right, intent->dir);

      break;
    }
  }
}

void test_correction_steady_20dg_to_far_right() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  csc.setInternalTarget(300);
  bool intentOccurred = false;
  uint32_t time = 0;

  for (int i = 0; i < config.regulations.observationBufferSize;
       i++) // Ein Buffer Simuliert
  {
    time += 1000; // 1 Hz Takt

    csc.currentHDG(320); // +20° Fehler konstant

    auto intent = csc.tick(time);

    if (intent.has_value()) {
      intentOccurred = true;

      TEST_ASSERT_EQUAL(SteeringDirection::Left, intent->dir);

      break;
    }
  }

  TEST_ASSERT_TRUE(intentOccurred);
}

void test_correction_over360_steady_20dg_to_far_left() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  csc.setInternalTarget(10);
  bool intentOccurred = false;
  uint32_t time = 0;

  for (int i = 0; i < 100; i++) {
    time += 1000;
    csc.currentHDG(350);

    auto intent = csc.tick(time);
    if (intent.has_value()) {
      TEST_ASSERT_EQUAL(SteeringDirection::Right, intent->dir);
      break;
    }
  }
};

void test_correction_over360_steady_20dg_to_far_right() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  csc.setInternalTarget(350);
  bool intentOccured = false;
  uint32_t time = 0;

  for (int i = 0; i < 100; i++) {
    time += 1000;
    csc.currentHDG(10);

    auto intent = csc.tick(time);
    if (intent.has_value()) {
      TEST_ASSERT_EQUAL(SteeringDirection::Left, intent->dir);
    }
  }
};

void test_correction_values_oscillating_to_zero_over_full_buffer() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  csc.setInternalTarget(300);
  uint16_t lastHDG = 0;
  bool intentOccured = false;
  uint32_t time = 0;

  /* Es wird abwechselnd ein Wert außerhalb der Toleranz in jede Richtung
  geschrieben, sodass sich diese ausmitteln sollten*/
  for (int i = 0; i < config.regulations.observationBufferSize; i++) {
    time += 1000;
    if (lastHDG != 320) {
      csc.currentHDG(320);
      lastHDG = 320;
    } else if (lastHDG != 280) {
      csc.currentHDG(280);
      lastHDG = 280;
    };

    auto intent = csc.tick(time);

    TEST_ASSERT_FALSE(intent.has_value());
  }
};

void test_no_correction_timer_guard() {
  SteeringController_Config config;
  CoreSteeringController csc(config);

  csc.setInternalTarget(300);
  bool intentOccured = false;
  uint32_t time = 0;
  uint8_t loopCounter = 0;

  for (int i = 0; i < config.regulations.observationBufferSize; i++) {
    time += 100;
    loopCounter++;
    csc.currentHDG(100);
    auto intent = csc.tick(time);

    if (intent.has_value()) {
      TEST_ASSERT_TRUE(loopCounter ==
                       10 * config.regulations.minimumTimeBtwObs_ms);
    }
  };
}

void test_no_correction_cooldown_guard() {
  SteeringController_Config config;
  config.regulations.minimumTimeBtwObs_ms = 1;
  config.regulations.pauseForValidObsAfterImpulse_ms = 0;
  config.regulations.SteeringCooldown_ms = 40;
  CoreSteeringController csc(config);

  csc.setInternalTarget(300);
  uint32_t time = 0;
  bool firstIntentSeen = false;
  uint32_t t0 = 0;

  // 1) Erstes Intent erzeugen (stetiger großer Fehler).
  for (int i = 0; i < 200; i++) {
    time += config.regulations.minimumTimeBtwObs_ms;
    csc.currentHDG(100);
    auto intent = csc.tick(time);
    if (intent.has_value()) {
      firstIntentSeen = true;
      t0 = time;
      break;
    }
  }
  TEST_ASSERT_TRUE(firstIntentSeen);

  // 2) Kurz vor Cooldown-Ende muss noch blockiert sein.
  csc.currentHDG(100);
  auto beforeCooldown =
      csc.tick(t0 + config.regulations.SteeringCooldown_ms - 1);
  TEST_ASSERT_FALSE(beforeCooldown.has_value());

  // 3) Direkt nach Cooldown-Ende muss wieder ein Intent moeglich sein.
  csc.currentHDG(100);
  auto afterCooldown =
      csc.tick(t0 + config.regulations.SteeringCooldown_ms + 1);
  TEST_ASSERT_TRUE(afterCooldown.has_value());
}

void test_no_correction_pause_for_valid_obs_guard() {
  SteeringController_Config config;
  config.regulations.minimumTimeBtwObs_ms = 1;
  config.regulations.SteeringCooldown_ms = 0;
  config.regulations.pauseForValidObsAfterImpulse_ms = 80;
  CoreSteeringController csc(config);

  csc.setInternalTarget(300);
  uint32_t time = 0;
  bool firstIntentSeen = false;
  uint32_t t0 = 0;

  // 1) Erstes Intent erzeugen.
  for (int i = 0; i < 200; i++) {
    time += config.regulations.minimumTimeBtwObs_ms;
    csc.currentHDG(100);
    auto intent = csc.tick(time);
    if (intent.has_value()) {
      firstIntentSeen = true;
      t0 = time;
      break;
    }
  }
  TEST_ASSERT_TRUE(firstIntentSeen);

  // 2) Kurz vor Ende der Beobachtungspause darf noch kein neues Intent kommen.
  csc.currentHDG(100);
  auto beforePauseEnd =
      csc.tick(t0 + config.regulations.pauseForValidObsAfterImpulse_ms - 1);
  TEST_ASSERT_FALSE(beforePauseEnd.has_value());

  // 3) Nach Ende der Pause Beobachtungen sammeln, dann muss wieder ein Intent
  // kommen.
  bool secondIntentSeen = false;
  time = t0 + config.regulations.pauseForValidObsAfterImpulse_ms + 1;
  for (int i = 0; i < 200; i++) {
    csc.currentHDG(100);
    auto intent = csc.tick(time);
    if (intent.has_value()) {
      secondIntentSeen = true;
      break;
    }
    time += config.regulations.minimumTimeBtwObs_ms;
  }
  TEST_ASSERT_TRUE(secondIntentSeen);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_no_correction_inside_tolerance);
  RUN_TEST(test_correction_steady_20dg_to_far_left);
  RUN_TEST(test_correction_steady_20dg_to_far_right);
  RUN_TEST(test_correction_over360_steady_20dg_to_far_left);
  RUN_TEST(test_correction_over360_steady_20dg_to_far_right);
  RUN_TEST(test_correction_values_oscillating_to_zero_over_full_buffer);
  RUN_TEST(test_no_correction_timer_guard);
  RUN_TEST(test_no_correction_cooldown_guard);
  RUN_TEST(test_no_correction_pause_for_valid_obs_guard);
  return UNITY_END();
}
