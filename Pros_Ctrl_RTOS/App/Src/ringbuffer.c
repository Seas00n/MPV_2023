/*
 * ringbuffer.c
 *
 *  Created on: Mar 23, 2023
 *      Author: root
 */
#include "ringbuffer.h"
#include "main.h"
#include "stdint.h"
static uint32_t i,j;
static int read_ok = 1;
volatile uint32_t write_residual = 0;
volatile uint32_t read_residual = 0;
void RingBuf_WriteByte(ringBuffer_t* buffer,uint8_t data){
	buffer->ringBuf[buffer->tailPosition]=data;
	if(++(buffer->tailPosition)>=Buffer_MAX){
		buffer->tailPosition = 0;
	}else{}
	if(buffer->tailPosition==buffer->headPosition){
		if(++buffer->headPosition>=Buffer_MAX){
			buffer->headPosition = 0;
		}
		write_residual+=1;
	}else{}
}
void RingBuf_WriteByteArray(ringBuffer_t* buffer, uint8_t* pData, uint32_t length){
	buffer->newest_frame_position = buffer->tailPosition;
	if(length+buffer->tailPosition>=Buffer_MAX){
		i = Buffer_MAX-buffer->tailPosition;
		memcpy((uint8_t*)(buffer->ringBuf)+buffer->tailPosition,pData,i);
		buffer->tailPosition = length-i;
		memcpy((uint8_t*)(buffer->ringBuf),(uint8_t*)pData+i,length-i);
	}else{
		memcpy((uint8_t*)(buffer->ringBuf)+buffer->tailPosition,pData,length);
      buffer->tailPosition = buffer->tailPosition+length;
	}
//	for(i=0;i<length;i++){
//		RingBuf_WriteByte(buffer, *(pData+i));
//	}
}
int RingBuf_ReadByte(ringBuffer_t* buffer, uint8_t* pData){
	if(buffer->headPosition==buffer->tailPosition){
		//read_residual+=1;
		return 0;
	}else{
		*pData = buffer->ringBuf[buffer->headPosition];
		if(++(buffer->headPosition)>=Buffer_MAX){
			buffer->headPosition = 0;
			return 1;
		}
	}
	return 1;
}

int RingBuf_ReadByteArray(ringBuffer_t* buffer, uint8_t* pData, uint32_t length){
	read_ok = 1;
	for(j=0;j<length;j++){
		read_ok *= RingBuf_ReadByte(buffer, (uint8_t*)(pData+j));
	}
//	if(buffer->headPosition+length>=Buffer_MAX){
//		i = Buffer_MAX-buffer->headPosition;
//		j = length-i;
//		if(i<=(buffer->tailPosition)||(buffer->tailPosition)<=j){
//			read_ok = 0;
//			return read_ok;
//		}else{
//			memcpy((uint8_t*)pData,(uint8_t*)(buffer->ringBuf)+buffer->headPosition,i);
//			buffer->headPosition = length-i;
//			memcpy((uint8_t*)pData+i,(uint8_t*)(buffer->ringBuf),length-i);
//			read_ok = 1;
//			return read_ok;
//		}
//	}else{
//		if(buffer->headPosition+length>buffer->tailPosition){
//			read_ok = 0;
//			return read_ok;
//		}else{
//			memcpy(pData,(uint8_t*)(buffer->ringBuf)+buffer->headPosition,length);
//			buffer->headPosition = buffer->headPosition+length;
//			read_ok = 1;
//			return read_ok;
//		}
//	}
	return read_ok;
}
int RingBuf_ReadByteNewestArray(ringBuffer_t* buffer,uint8_t* pData, uint32_t length){
	buffer->headPosition = buffer->newest_frame_position;
	read_ok = RingBuf_ReadByteArray(buffer,pData,length);
	return read_ok;
}
