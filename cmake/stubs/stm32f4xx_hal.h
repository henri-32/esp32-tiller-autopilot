#pragma once

#include <cstdint>

using HAL_StatusTypeDef = int;

constexpr HAL_StatusTypeDef HAL_OK = 0;

struct RCC_PLLInitTypeDef
{
    std::uint32_t PLLState = 0;
    std::uint32_t PLLSource = 0;
    std::uint32_t PLLM = 0;
    std::uint32_t PLLN = 0;
    std::uint32_t PLLP = 0;
    std::uint32_t PLLQ = 0;
};

struct RCC_OscInitTypeDef
{
    std::uint32_t OscillatorType = 0;
    std::uint32_t HSIState = 0;
    std::uint32_t HSICalibrationValue = 0;
    RCC_PLLInitTypeDef PLL{};
};

struct RCC_ClkInitTypeDef
{
    std::uint32_t ClockType = 0;
    std::uint32_t SYSCLKSource = 0;
    std::uint32_t AHBCLKDivider = 0;
    std::uint32_t APB1CLKDivider = 0;
    std::uint32_t APB2CLKDivider = 0;
};

constexpr std::uint32_t PWR_REGULATOR_VOLTAGE_SCALE1 = 0;
constexpr std::uint32_t RCC_OSCILLATORTYPE_HSI = 0;
constexpr std::uint32_t RCC_HSI_ON = 0;
constexpr std::uint32_t RCC_HSICALIBRATION_DEFAULT = 0;
constexpr std::uint32_t RCC_PLL_ON = 0;
constexpr std::uint32_t RCC_PLLSOURCE_HSI = 0;
constexpr std::uint32_t RCC_PLLP_DIV4 = 0;
constexpr std::uint32_t RCC_CLOCKTYPE_SYSCLK = 1u << 0;
constexpr std::uint32_t RCC_CLOCKTYPE_HCLK = 1u << 1;
constexpr std::uint32_t RCC_CLOCKTYPE_PCLK1 = 1u << 2;
constexpr std::uint32_t RCC_CLOCKTYPE_PCLK2 = 1u << 3;
constexpr std::uint32_t RCC_SYSCLKSOURCE_PLLCLK = 0;
constexpr std::uint32_t RCC_SYSCLK_DIV1 = 0;
constexpr std::uint32_t RCC_HCLK_DIV2 = 0;
constexpr std::uint32_t RCC_HCLK_DIV1 = 0;
constexpr std::uint32_t FLASH_LATENCY_2 = 0;

static inline HAL_StatusTypeDef HAL_Init()
{
    return HAL_OK;
}

static inline HAL_StatusTypeDef HAL_RCC_OscConfig(const RCC_OscInitTypeDef*)
{
    return HAL_OK;
}

static inline HAL_StatusTypeDef HAL_RCC_ClockConfig(const RCC_ClkInitTypeDef*, std::uint32_t)
{
    return HAL_OK;
}

static inline std::uint32_t HAL_GetTick()
{
    return 0;
}

static inline void HAL_Delay(std::uint32_t)
{
}

static inline void __disable_irq()
{
}

#define __HAL_RCC_PWR_CLK_ENABLE() ((void)0)
#define __HAL_PWR_VOLTAGESCALING_CONFIG(...) ((void)0)
