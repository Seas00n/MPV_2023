/*
 * msg.h
 *
 *  Created on: Mar 14, 2023
 *      Author: root
 */

#ifndef INC_MSG_H_
#define INC_MSG_H_
typedef struct{
	uint8_t head;
	uint8_t end;
	uint16_t value1;
	uint16_t value2;
	uint16_t value3;
	uint16_t value4;
	uint16_t value5;
	uint16_t value6;
}M2P;
typedef struct{
	uint8_t head;
	uint8_t end;
	uint8_t id;
	uint16_t value1;
	uint16_t value2;
	uint16_t value3;
	uint16_t value4;
	uint8_t ext_value;
}P2M;
void p2mmsg_memcpy(uint8_t*,volatile P2M);
void m2pmsg_memcpy(uint8_t*,volatile M2P);
#endif /* INC_MSG_H_ */
