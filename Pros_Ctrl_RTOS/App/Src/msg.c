/*
 * msg.c
 *
 *  Created on: Mar 15, 2023
 *      Author: root
 */
#include "main.h"
#include "msg.h"

float k_float2int16 = 300;
float b_float2int16 = 30000;
float k_float2int12 = 22;
float b_float2int12 = 2110;

void p2mmsg_memcpy(uint8_t* buffer,volatile P2M p2m){
	//TODO
}
void m2pmsg_memcpy(uint8_t* buffer,volatile M2P m2p){
	buffer[0] = m2p.head;
	buffer[1] = m2p.value1>>8&0xff;
	buffer[2] = m2p.value1&0xff;
	buffer[3] = m2p.value2>>8&0xff;
	buffer[4] = m2p.value2&0xff;
	buffer[5] = m2p.value3>>8&0xff;
	buffer[6] = m2p.value3&0xff;
	buffer[7] = m2p.value4>>8&0xff;
	buffer[8] = m2p.value4&0xff;
	buffer[9] = m2p.value5>>8&0xff;
	buffer[10] = m2p.value5&0xff;
	buffer[11] = m2p.value6>>8&0xff;
	buffer[12] = m2p.value6&0xff;
	buffer[13] = m2p.end;
}

