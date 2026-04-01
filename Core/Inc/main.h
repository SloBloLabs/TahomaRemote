/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void sendCommand(const char* buf, size_t len);
size_t available(void);
char nextChar();
size_t readRingBuffer(char* buf);
void uartRxCallback();
void uartTxCompleteCallback(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BTN0_Pin LL_GPIO_PIN_0
#define BTN0_GPIO_Port GPIOC
#define BTN0_EXTI_IRQn EXTI0_IRQn
#define BTN1_Pin LL_GPIO_PIN_1
#define BTN1_GPIO_Port GPIOC
#define BTN1_EXTI_IRQn EXTI1_IRQn
#define BTN2_Pin LL_GPIO_PIN_2
#define BTN2_GPIO_Port GPIOC
#define BTN2_EXTI_IRQn EXTI2_IRQn
#define BTN3_Pin LL_GPIO_PIN_3
#define BTN3_GPIO_Port GPIOC
#define BTN3_EXTI_IRQn EXTI3_IRQn
#define LED_Pin LL_GPIO_PIN_3
#define LED_GPIO_Port GPIOA
#define BTN4_Pin LL_GPIO_PIN_4
#define BTN4_GPIO_Port GPIOC
#define BTN4_EXTI_IRQn EXTI4_IRQn
#define BTN5_Pin LL_GPIO_PIN_5
#define BTN5_GPIO_Port GPIOC
#define BTN5_EXTI_IRQn EXTI9_5_IRQn
#define BTN6_Pin LL_GPIO_PIN_6
#define BTN6_GPIO_Port GPIOC
#define BTN6_EXTI_IRQn EXTI9_5_IRQn
#define BTN7_Pin LL_GPIO_PIN_7
#define BTN7_GPIO_Port GPIOC
#define BTN7_EXTI_IRQn EXTI9_5_IRQn
#define BTN8_Pin LL_GPIO_PIN_8
#define BTN8_GPIO_Port GPIOC
#define BTN8_EXTI_IRQn EXTI9_5_IRQn
#define LED_OEN_Pin LL_GPIO_PIN_15
#define LED_OEN_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
