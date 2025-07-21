#ifndef USART_CONFIG_H
#define USART_CONFIG_H

#include <stm32l0xx.h> 

void USART2_Init(uint32_t baudrate);


bool USART2_read(char *caracter);
void USART2_write(uint8_t c);
void USART2_Write_String(const char* string);
#endif