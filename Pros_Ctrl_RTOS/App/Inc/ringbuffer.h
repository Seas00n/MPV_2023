/*
 * ringbuffer.h
 *
 *  Created on: Mar 23, 2023
 *      Author: root
 */

#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_
#define Buffer_MAX 21
#include "main.h"
#include "stdio.h"
typedef struct{
	uint8_t headPosition;
	uint8_t tailPosition;
	uint8_t ringBuf[Buffer_MAX];
	uint8_t newest_frame_position;
}ringBuffer_t;
void RingBuf_WriteByte(ringBuffer_t* buffer,uint8_t data);
void RingBuf_WriteByteArray(ringBuffer_t* buffer, uint8_t* pData, uint32_t length);
int RingBuf_ReadByte(ringBuffer_t* buffer, uint8_t* pData);
int RingBuf_ReadByteArray(ringBuffer_t* buffer, uint8_t* pData, uint32_t length);
int RingBuf_ReadByteNewestArray(ringBuffer_t* buffer,uint8_t* pData, uint32_t length);




#endif /* INC_RINGBUFFER_H_ */
