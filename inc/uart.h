#ifndef __UART_H
#define __UART_H

#include "stm32f4xx_hal.h"

#define UART_DELAY 100

// itoa is available but never declared.
// We declare it here to silence the compiler.
char* itoa(int, char*, int);

extern UART_HandleTypeDef uart;

void UART_Init(void);

void print(char *s);
void print_char(char *c);
void print_int(int i, uint8_t base);

HAL_StatusTypeDef input(char *buf, uint16_t len);

#endif // __UART_H
