/*
 * debug_uart.c
 *
 *  Created on: Mar 14, 2023
 *      Author: root
 */
#include "main.h"
#include "debug_uart.h"
#include "stdio.h"
#include "stdarg.h"
static char tempBuf[40];
extern UART_HandleTypeDef huart8;
void debugPrint(char *fmt,...){
	//可变长度参数
	va_list argp;
	uint32_t n = 0;
	// 以fmt为起始地址，获取第一个参数的首地址
	va_start(argp, fmt);
	n = vsprintf((char *) tempBuf, fmt, argp);
	// argp置0
	va_end(argp);
	if (HAL_UART_Transmit(&huart8, (uint8_t *) tempBuf, n, 1000) != HAL_OK) {
		/* Transfer error in transmission process */
		Error_Handler();
	}
}



