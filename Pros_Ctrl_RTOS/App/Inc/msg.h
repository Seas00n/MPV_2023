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
	uint16_t value7;
	uint16_t value8;
}M2P;//size 18
typedef struct{
	uint8_t head;
	uint8_t end;
	uint8_t id;
	uint16_t value1;
	uint16_t value2;
	uint16_t value3;
	uint16_t value4;
	uint16_t value5;
	uint16_t value6;
}P2M;//size 15
void p2mmsg_memcpy(uint8_t*,volatile P2M);
void m2pmsg_memcpy(uint8_t*,volatile M2P);
#endif /* INC_MSG_H_ */
