#include "drivers/pwmDriver.h"
#include "driver/ledc.h"
#include "types/steeringTypes.h"
#include <cstdio>

void PwmDriver::command(const PWMIntent& command)
{
  // TODO HardwareImpulse ausgeben

  const auto dir = command.dir;

  uint32_t pulse_ms = SteeringMechanicsConfig::steeringMinImpulse_ms +
                      (command.filteredAbstractImpulse_0_100 / 100.0f) *
                          (SteeringMechanicsConfig::steeringMaxImpulse_ms -
                           SteeringMechanicsConfig::steeringMinImpulse_ms);
}

esp_err_t PwmDriver::init()
{

  ledc_timer_config_t timer_cfg = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                   .duty_resolution = LEDC_TIMER_8_BIT,
                                   .timer_num = LEDC_TIMER_0,
                                   .freq_hz = 20'000,
                                   .clk_cfg = LEDC_AUTO_CLK};

  if (ledc_timer_config(&timer_cfg) != ESP_OK)
  {
    return ESP_FAIL;
  };

  ledc_channel_config_t channel_cfg = {.gpio_num = GPIO_NUM_17,
                                       .speed_mode = LEDC_LOW_SPEED_MODE,
                                       .channel = LEDC_CHANNEL_0,
                                       .timer_sel = LEDC_TIMER_0,
                                       .duty = 0,
                                       .hpoint = 0,
                                       .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD};

  ledc_channel_config_t channel_cfg2 = {.gpio_num = GPIO_NUM_18,
                                        .speed_mode = LEDC_LOW_SPEED_MODE,
                                        .channel = LEDC_CHANNEL_1,
                                        .timer_sel = LEDC_TIMER_0,
                                        .duty = 0,
                                        .hpoint = 0,
                                        .sleep_mode =LEDC_SLEEP_MODE_NO_ALIVE_NO_PD};

  if (ledc_channel_config(&channel_cfg) != ESP_OK | ledc_channel_config(&channel_cfg2) != ESP_OK)
  {
    return ESP_FAIL;
  }

  return ESP_OK;
}
