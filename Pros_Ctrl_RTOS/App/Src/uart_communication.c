/*
 * uart_communication.c
 *
 *  Created on: Feb 22, 2023
 *      Author: yuxuan
 */

#include "main.h"
#include "uart_communication.h"
#include "ringbuffer.h"
#include "msg.h"
#include "debug_uart.h"
#include "stdio.h"
#include "stdarg.h"
#include "cmsis_os.h"




static uint8_t txDataBuffer[40];
static uint8_t rxDataBuffer[40];
static uint8_t rxData[11];
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

volatile P2M p2m_pc = {0x01,0xFF,0x00,0x0000,0x0000,0x0000,0x0000,0x00};//
volatile M2P m2p_pc = {0x01,0xFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};//
extern volatile P2M p2m;
extern volatile M2P m2p;
extern osMutexId p2mMutexHandle;
extern osMutexId m2pMutexHandle;

static uint32_t size_m2p = 14;
static uint32_t size_p2m = 12;


static ringBuffer_t buffer;





//TODO
//Use to Debug
volatile float pos_desired_rtpc = 0;
volatile float pos_actual_rtpc = 0;
uint32_t msg_receive;
uint32_t msg_send;

void Start_PCReceiveIT(){
	HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rxDataBuffer, sizeof(rxData));
//	HAL_UART_Receive_DMA(&huart6, rxDataBuffer,11);
	__HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
}


//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
//	if(huart==&huart6){
//		msg_send +=1;
////		RingBuf_WriteByteArray(&buffer, rxDataBuffer, Size);
//		if(Size==11&&rxDataBuffer[0]==0xfc&&((rxDataBuffer[10]&0xf)==0xf)){
//			msg_receive += Size/11;
//			memcpy(rxData,rxDataBuffer,Size);
//		}
//		HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rxDataBuffer, sizeof(rxData));
//		__HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
//	}
//}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size){
	if(huart==&huart6){
		RingBuf_WriteByteArray(&buffer, rxDataBuffer, Size);
		RingBuf_ReadByteNewestArray(&buffer, rxData, sizeof(rxData));
		tx_len = size_m2p;
		HAL_UART_Transmit_DMA(&huart6, txDataBuffer,tx_len);
//		HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rxDataBuffer, sizeof(rxData));
//		__HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
	}

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart){
	if(huart==&huart6){
		msg_send+=1;
		if(rxDataBuffer[0]==0xfc&&((rxDataBuffer[10]&0xf)==0xf)){
			msg_receive +=1;
			memcpy(rxData,rxDataBuffer,sizeof(rxData));
			PC_UnpackMessage();
			PC_PackMessage();
		}
		HAL_UART_Transmit_DMA(&huart6, txDataBuffer, 14);
//		HAL_UART_Receive_DMA(&huart6, rxDataBuffer,11);
	}
}
void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart){
	huart->gState = HAL_UART_STATE_READY;
}
// [0]0x(id)* ,[9]0x*F
void PC_UnpackMessage(){
//	rx_len = 11;
//	if(RingBuf_ReadByteArray(&buffer, rxData, rx_len)==1){
//		if(rxData[0]==0xfc){
//			uint8_t* rxDataBuffer_ = rxData+1;
//			if(rx_len>=10 &&((rxDataBuffer_[9]&0xf)==0xf)){
//				p2m.head = 0x01;
//				p2m.id = (uint8_t)(rxDataBuffer_[0]>>4&0xf);
//				p2m.value1 = (uint16_t)(rxDataBuffer_[1]<<8|rxDataBuffer_[2]);
//				p2m.value2 = (uint16_t)(rxDataBuffer_[3]<<8|rxDataBuffer_[4]);
//				p2m.value3 = (uint16_t)(rxDataBuffer_[5]<<8|rxDataBuffer_[6]);
//				p2m.value4 = (uint16_t)(rxDataBuffer_[7]<<8|rxDataBuffer_[8]);
//				p2m.ext_value = (uint8_t)(((rxDataBuffer_[0]&0xf)<<4)|(rxDataBuffer_[9]>>4&0xf));
//				p2m.head = 0xFC;
//				pos_desired_rtpc = (float)((p2m.value1-b_float2int16)/k_float2int16);
//
//			}else{}
//		}else{}
//	}
	if(rxData[0]==0xfc&&rxData[0]==0xfc&&((rxData[10]&0xf)==0xf)){
		p2m.head = 0x01;
		p2m.id = (uint8_t)(rxData[1]>>4&0xf);
		p2m.value1 = (uint16_t)(rxData[2]<<8|rxData[3]);
		p2m.value2 = (uint16_t)(rxData[4]<<8|rxData[5]);
		p2m.value3 = (uint16_t)(rxData[6]<<8|rxData[7]);
		p2m.value4 = (uint16_t)(rxData[8]<<8|rxData[9]);
		p2m.ext_value = (uint8_t)(((rxData[1]&0xf)<<4)|(rxData[10]>>4&0xf));
		p2m.head = 0xFC;
		pos_desired_rtpc = (float)((p2m.value1-b_float2int16)/k_float2int16);
	}
}

void PC_PackMessage(){
//	if(m2p_pc.head==0xFC && m2p_pc.end==0xFF){
//		tx_len = size_m2p;
//		pos_actual_rtpc = (float)(((m2p_pc.value1>>4)&0xfff)-b_float2int12)/k_float2int12;
//		m2pmsg_memcpy(txDataBuffer, m2p_pc);
//		HAL_UART_Transmit_DMA(&huart6, txDataBuffer,tx_len);
//		count = 0;
//		while(huart6.gState!=HAL_UART_STATE_READY){
//			if(count>10){
//				PC_CommunicationErrorHandler();
//				break;
//			}else{
//				osDelay(1);
//				count+=1;
//			}
//		}
//	}
	if(m2p.head==0xFC && m2p.end==0xFF){
			pos_actual_rtpc = (float)(((m2p.value1>>4)&0xfff)-b_float2int12)/k_float2int12;
			m2pmsg_memcpy(txDataBuffer, m2p);
			tx_len = size_m2p;
//			HAL_UART_Transmit_DMA(&huart6, txDataBuffer,tx_len);
//			count = 0;
//			while(huart6.gState!=HAL_UART_STATE_READY){
//				if(count>1000){
//					PC_CommunicationErrorHandler();
//					break;
//				}else{
//					count+=1;
//				}
//			}
		}
}

void PC_CommunicationErrorHandler(){
	debugPrint("Send to PC Failed\r\n");
}
