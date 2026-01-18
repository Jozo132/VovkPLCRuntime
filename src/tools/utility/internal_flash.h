#pragma once

#include <Arduino.h>

#if defined(STM32) || defined(ARDUINO_ARCH_STM32)

// STM32 Flash utility - tested with STM32F4 devices
// Based on InternalStorage by Juraj Andrassy

// Include the appropriate HAL header
#if defined(STM32F4xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F1xx)
#include "stm32f1xx_hal.h"
#elif defined(STM32F2xx)
#include "stm32f2xx_hal.h"
#elif defined(STM32F3xx)
#include "stm32f3xx_hal.h"
#elif defined(STM32F7xx)
#include "stm32f7xx_hal.h"
#endif

#endif // STM32