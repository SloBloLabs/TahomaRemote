#include "Dbg.h"
#include <stdarg.h>
#include <string.h>
#include "stm32f4xx_ll_usart.h"
#include "FreeRTOS.h"
#include "semphr.h"
//#include "usbd_def.h"
//#include "usbd_cdc_if.h"

#define DEBUG_USART USART2

//static char printBuffer[DBG_PRINTBUFFER_SIZE];
static SemaphoreHandle_t uartMutex = NULL;

void uartDebug(char* msg, int len) {
    // Create mutex on first call
    if (uartMutex == NULL) {
        uartMutex = xSemaphoreCreateMutex();
    }
    
    // Acquire mutex to prevent concurrent UART access
    if (xSemaphoreTake(uartMutex, portMAX_DELAY) == pdTRUE) {
        size_t i = 0;
        while(i < len) {
            // Wait for TX buffer empty
            while(!LL_USART_IsActiveFlag_TXE(DEBUG_USART));
            LL_USART_TransmitData8(DEBUG_USART, msg[i++]);
        }
        // Wait for last character to finish transmitting
        while(!LL_USART_IsActiveFlag_TC(DEBUG_USART));
        xSemaphoreGive(uartMutex);
    }
}

/*
void DBG(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(printBuffer, DBG_PRINTBUFFER_SIZE, fmt, args);
    va_end(args);
    // Send the formatted string to the desired output (e.g., UART, ITM)
    // Here we use ITM for demonstration
    for (char* p = printBuffer; *p != '\0'; p++) {
        ITM_SendChar(*p);
    }
    //while(CDC_Transmit_FS((uint8_t*)printBuffer, strlen(printBuffer)) != HAL_OK);
    uartDebug(printBuffer, strlen(printBuffer));
}
*/

int _write(int file, char *ptr, int len) {
    //while(CDC_Transmit_FS((uint8_t *)ptr, len) != HAL_OK) {};
    uartDebug(ptr, len);
	return len;
}