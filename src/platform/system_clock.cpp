
#include "stm32f4xx_hal.h" // IWYU pragma: keep
#include "platform/platform.h"

void SystemClock_Config()
{
    RCC_OscInitTypeDef osc{};
    RCC_ClkInitTypeDef clk{};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    osc.PLL.PLLM = 16;
    osc.PLL.PLLN = 336;
    osc.PLL.PLLP = RCC_PLLP_DIV4;   // 84 MHz
    osc.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        FatalError("Oscillator config failed");
    }

    clk.ClockType =
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_HCLK   |
        RCC_CLOCKTYPE_PCLK1  |
        RCC_CLOCKTYPE_PCLK2;

    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2; // 42 MHz
    clk.APB2CLKDivider = RCC_HCLK_DIV1; // 84 MHz

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK)
    {
        FatalError("Clock config failed");
    }
}
