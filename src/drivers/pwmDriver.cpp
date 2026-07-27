#include "drivers/pwmDriver.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "types/steeringTypes.h"

void PwmDriver::command(const PWMIntent& cmd)
//{{{
/* This function should only be called from the systemController so that its running in the steering
 * Task*/
{

  uint8_t mapIntentToDuty =
      static_cast<uint8_t>((cmd.filteredAbstractImpulse_0_100 * 255) / 100);

  if (cmd.dir == SteeringDirection::PORTSIDE && starbordDuty_ == 0)
  {
    portsideDuty_ = mapIntentToDuty;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(portsideChannel_),
                  mapIntentToDuty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(portsideChannel_));
  }
  else if (cmd.dir == SteeringDirection::STARBOARD && portsideDuty_ == 0)
  {
    starbordDuty_ = mapIntentToDuty;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(starbordChannel_),
                  mapIntentToDuty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(starbordChannel_));
  }
  else
  {
    return;
  }

  // This TaskDelay practically sets the duration of the steering intent, as no update of the ledc
  // controller happens for this time, so the set duty remains aktive as long as the task is blocked

  vTaskDelay(pdMS_TO_TICKS(cmd.duration));

  // TODO At the Moment every command has a duration so after this time is over the command is
  // consumed and the dutys should be zero. In the future it might be a constant steering over a
  // continous updata of the duty cycle. In this case the pwm controller only steers the current
  // command
  ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(portsideChannel_), 0);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(starbordChannel_), 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(portsideChannel_));
  ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(starbordChannel_));

  portsideDuty_ = 0;
  starbordDuty_ = 0;
}
//}}}

esp_err_t PwmDriver::init()
//{{{
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
                                        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD};

  // Defines which channel steers in which direction
  portsideChannel_ = LEDC_CHANNEL_0;
  starbordChannel_ = LEDC_CHANNEL_1;

  if (ledc_channel_config(&channel_cfg) != ESP_OK | ledc_channel_config(&channel_cfg2) != ESP_OK)
  {
    return ESP_FAIL;
  }

  return ESP_OK;
}
//}}}
