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

volatile P2M p2m = {0x01,0xFF,0x00,0x0000,0x0000,0x0000,0x0000,0x00};//
volatile M2P m2p = {0x01,0xFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};//

void p2mmsg_memcpy(uint8_t* buffer,volatile P2M p2m_){
	//TODO
}
void m2pmsg_memcpy(uint8_t* buffer,volatile M2P m2p_){
	buffer[0] = m2p_.head;
	buffer[1] = m2p_.value1>>8&0xff;
	buffer[2] = m2p_.value1&0xff;
	buffer[3] = m2p_.value2>>8&0xff;
	buffer[4] = m2p_.value2&0xff;
	buffer[5] = m2p_.value3>>8&0xff;
	buffer[6] = m2p_.value3&0xff;
	buffer[7] = m2p_.value4>>8&0xff;
	buffer[8] = m2p_.value4&0xff;
	buffer[9] = m2p_.value5>>8&0xff;
	buffer[10] = m2p_.value5&0xff;
	buffer[11] = m2p_.value6>>8&0xff;
	buffer[12] = m2p_.value6&0xff;
	buffer[13] = m2p_.value7>>8&0xff;
	buffer[14] = m2p_.value7&0xff;
	buffer[15] = m2p_.value8>>8&0xff;
	buffer[16] = m2p_.value8&0xff;
	buffer[17] = m2p_.end;
}

