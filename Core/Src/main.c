/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Dbg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ESP_UART USART1
#define ESP_DMA DMA2
#define ESP_RX_DMA_STREAM LL_DMA_STREAM_5
#define ESP_TX_DMA_STREAM LL_DMA_STREAM_7
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Buffer used for transmission */
#define RX_BUFFER_SIZE 1024
char rxBuffer[RX_BUFFER_SIZE];
volatile size_t readPtr = 0;
volatile size_t writePtr = 0;

#define TX_BUFFER_SIZE 1024
char txBuffer[TX_BUFFER_SIZE];
volatile uint8_t transmissionComplete = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void     setupDMA(void);
int      StartTransfers(void);
uint8_t  Buffercmp8(uint8_t* pBuffer1, uint8_t* pBuffer2, uint8_t BufferLength);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  setupDMA();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_5)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 168, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(168000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void setupDMA() {

  // Enable RX interrupts (HT, TC, Usart IDLE)
  LL_DMA_EnableIT_HT(ESP_DMA, ESP_RX_DMA_STREAM);
  LL_DMA_EnableIT_TC(ESP_DMA, ESP_RX_DMA_STREAM);
  LL_USART_EnableIT_IDLE(ESP_UART);

  // Enable DMA TX TC interrupts
  LL_DMA_EnableIT_TC(ESP_DMA, ESP_TX_DMA_STREAM);

  // Configure TX DMA
  LL_DMA_SetPeriphAddress(ESP_DMA, ESP_TX_DMA_STREAM, LL_USART_DMA_GetRegAddr(USART1));

  // start DMA reception
  LL_DMA_SetPeriphAddress(ESP_DMA, ESP_RX_DMA_STREAM, LL_USART_DMA_GetRegAddr(USART1));
  LL_DMA_SetMemoryAddress(ESP_DMA, ESP_RX_DMA_STREAM, (uint32_t)rxBuffer);
  LL_DMA_SetDataLength(ESP_DMA, ESP_RX_DMA_STREAM, sizeof(rxBuffer));
  LL_USART_EnableDMAReq_RX(ESP_UART);
  //LL_USART_ClearFlag_TC(USART1);
  LL_DMA_EnableStream(ESP_DMA, ESP_RX_DMA_STREAM);

}

void sendCommand(const char* buf, size_t len) {
  TRACE_LOG("sendCommand: %.*s with length %d\n", len, buf, len);
  if(len >= TX_BUFFER_SIZE) {
    USER_LOG("Command too long for TR Buffer %d", len);
    return;
  }
  
  while(transmissionComplete != 1);
  memcpy(txBuffer, buf, len);

  // DMA transfer
  LL_DMA_SetMemoryAddress(ESP_DMA, ESP_TX_DMA_STREAM, (uint32_t)txBuffer);
  LL_DMA_SetDataLength(ESP_DMA, ESP_TX_DMA_STREAM, len);
  LL_USART_EnableDMAReq_TX(ESP_UART);
  LL_USART_ClearFlag_TC(ESP_UART);
  LL_DMA_EnableStream(ESP_DMA, ESP_TX_DMA_STREAM);
  transmissionComplete = 0;
}

size_t available() {
  if(writePtr >= readPtr) {
    return writePtr - readPtr;
  } else {
    return (sizeof(rxBuffer) - readPtr) + writePtr;
  }
}

char nextChar() {
  if(!available()) {
    return 0;
  }
  char c = rxBuffer[readPtr++];
  if(readPtr == RX_BUFFER_SIZE) {
    readPtr = 0;
  }
  return c;
}

void uartRxCallback() {
  writePtr = sizeof(rxBuffer) - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_5);
}

void uartTxCompleteCallback() {
  transmissionComplete = 1;
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
