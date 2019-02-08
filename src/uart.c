#include "uart.h"
#include <string.h>

UART_HandleTypeDef uart;

void UART_Init(void) {
  uart.Instance = USART3;
  uart.Init.BaudRate = 115200;
  uart.Init.WordLength = UART_WORDLENGTH_8B;
  uart.Init.StopBits = UART_STOPBITS_1;
  uart.Init.Parity = UART_PARITY_NONE;
  uart.Init.Mode = UART_MODE_TX_RX;
  uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  uart.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&uart) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void print(char *s) {
  HAL_UART_Transmit(&uart, (uint8_t*) s, strlen(s), UART_DELAY);
}

void print_char(char *c) {
  HAL_UART_Transmit(&uart, (uint8_t*) c, 1, UART_DELAY);
}

HAL_StatusTypeDef input(char *buf, uint16_t len) {
  return HAL_UART_Receive(&uart, (uint8_t*) buf, len, UART_DELAY);
}

void print_int(int i, uint8_t base) {
  char buf[(sizeof(uint32_t)*8+1)];
  itoa(i, buf, base);
  HAL_UART_Transmit(&uart, (uint8_t*) buf, strlen(buf), UART_DELAY);
}

