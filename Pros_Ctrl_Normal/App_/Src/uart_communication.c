/*
 * uart_communication.c
 *
 *  Created on: Feb 22, 2023
 *      Author: yuxuan
 */

#include "main.h"
#include "uart_communication.h"
#include "msg.h"
#include "debug_uart.h"
#include "stdio.h"
#include "stdarg.h"



static uint8_t txDataBuffer[40];
static uint8_t rxDataBuffer[40];
static uint32_t rx_len;
static uint32_t tx_len;

extern float k_float2int16;
extern float b_float2int16;
extern float k_float2int12;
extern float b_float2int12;

static uint8_t count = 0;

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;

extern volatile P2M p2m;//
extern volatile M2P m2p;//
static uint32_t size_m2p = 14;
static uint32_t size_p2m = 12;


//TODO
//Use to Debug
float pos_desired_rtpc = 0;
float pos_actual_rtpc = 0;

//TODO
uint8_t sbus_rx_buffer[2][10];


void Start_PCReceiveIT(){
//	HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rxDataBuffer, sizeof(rxDataBuffer));
	HAL_UART_Receive_DMA(&huart6, rxDataBuffer,10);
}



//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
//	if(huart==&huart6){
//		rx_len = Size;
//		PC_UnpackMessage();
////		while(HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rxDataBuffer, sizeof(rxDataBuffer))!=HAL_OK){
////			HAL_UART_DMAStop(&huart6);
////		}
////		__HAL_DMA_DISABLE_IT(&hdma_usart6_rx,DMA_IT_HT);
////		HAL_UART_Transmit_DMA(&huart6, txDataBuffer,tx_len);
////		HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rxDataBuffer, sizeof(rxDataBuffer));
////		if(txDataBuffer[0]==0xFC&&txDataBuffer[size_m2p-1]==0xFF){
////		}else{}
//	}
//}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart){
	rx_len = 20;
	PC_UnpackMessage();
}


void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart){
	huart->gState = HAL_UART_STATE_READY;
}
// [0]0x(id)* ,[9]0x*F
void PC_UnpackMessage(){
	if((rx_len>=10) && ((rxDataBuffer[9]&0xf)==0xf)){
		p2m.head = 0x01;
		p2m.id = (uint8_t)(rxDataBuffer[0]>>4&0xf);
		p2m.value1 = (uint16_t)(rxDataBuffer[1]<<8|rxDataBuffer[2]);
		p2m.value2 = (uint16_t)(rxDataBuffer[3]<<8|rxDataBuffer[4]);
		p2m.value3 = (uint16_t)(rxDataBuffer[5]<<8|rxDataBuffer[6]);
		p2m.value4 = (uint16_t)(rxDataBuffer[7]<<8|rxDataBuffer[8]);
		p2m.ext_value = (uint8_t)(((rxDataBuffer[0]&0xf)<<4)|(rxDataBuffer[9]>>4&0xf));
		p2m.head = 0xFC;
		pos_desired_rtpc = (float)((p2m.value1-b_float2int16)/k_float2int16);
	}else{}
}

void PC_PackMessage(){
	if(m2p.head==0xFC && m2p.end==0xFF){
		tx_len = size_m2p;
		pos_actual_rtpc = (float)(((m2p.value1>>4)&0xfff)-b_float2int12)/k_float2int12;
		m2pmsg_memcpy(txDataBuffer, m2p);
		HAL_UART_Transmit_DMA(&huart6, txDataBuffer,tx_len);
		count = 0;
		while(huart6.gState!=HAL_UART_STATE_READY){
			if(count>10){
				PC_CommunicationErrorHandler();
				break;
			}else{
				HAL_Delay(2);
				count+=1;
			}
		}
	}
}

void PC_CommunicationErrorHandler(){
	debugPrint("Send to PC Failed\r\n");
}