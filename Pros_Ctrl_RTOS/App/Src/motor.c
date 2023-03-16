/*
 * motor.c
 *
 *  Created on: Feb 22, 2023
 *      Author: yuxuan
 */
#include "main.h"
#include "motor.h"
#include "uart_communication.h"
#include "msg.h"
#include "debug_uart.h"
extern CAN_HandleTypeDef hcan1;

static uint8_t txDataBuffer[10];
static uint8_t rxDataBuffer[10];

CAN_TxHeaderTypeDef txHeader;
CAN_RxHeaderTypeDef rxHeader;

uint32_t txMailBox;

static uint8_t send_idx;

static uint16_t msg_ext_id;

typedef enum{
	CAN_PACKET_SET_DUTY = 0,
	//占空比模式
	CAN_PACKET_SET_CURRENT,
	//电流环模式
	CAN_PACKET_SET_CURRENT_BRAKE,
	// 电流刹车模式
	CAN_PACKET_SET_RPM,
	// 转速模式
	CAN_PACKET_SET_POS,
	// 位置模式
	CAN_PACKET_SET_ORIGIN_HERE,
	//设置原点模式
	CAN_PACKET_SET_POS_SPD,
	//位置速度环模式
}CAN_PACKET_ID;

volatile MotorTypeDef motor_knee = {0,0,0,0,0,0,0,0,0,0,1};
volatile MotorTypeDef motor_ankle = {0,0,0,0,0,0,0,0,0,0,1};
extern float k_float2int16;
extern float b_float2int16;
extern float k_float2int12;
extern float b_float2int12;

static int16_t pos_int;
static int16_t vel_int;
static int16_t cur_int;
static int16_t temp_int;
#define RADs_TO_ERPM(v) (v*30.0/3.14159265359*21.0*64.0)
#define ERPM_TO_RADs(v) (v/21.0/64.0*3.14159265359/30.0)
#define MIN(X,Y) ({\
	typeof (X) x_ = (X);\
	typeof (Y) y_ = (Y);\
	(x_ < y_) ? x_ : y_; })
#define MAX(X,Y) ({\
	typeof (X) x_ = (X);\
	typeof (Y) y_ = (Y);\
	(x_ > y_) ? x_ : y_; })


volatile M2P m2p_motor = {0x01,0xFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,};
volatile P2M p2m_motor = {0x01,0xFF,0x00,0x0000,0x0000,0x0000,0x0000,0x00,};


static float pos_max = 90.0;
static float vel_max = ERPM_TO_RADs(80000);
static float current_max = 20.0;
static float torque_max = 144.0;
static float accel_max = 30000;

void CAN_FilterConfig(){
	CAN_FilterTypeDef sFilterConfig;
	sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x00;
	sFilterConfig.FilterIdLow  = 0x00;
	if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig)!=HAL_OK){
		Error_Handler();
	}
	if(HAL_CAN_Start(&hcan1)!=HAL_OK){
		Error_Handler();
	}
	if(HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING|CAN_IT_RX_FIFO1_MSG_PENDING)){
		Error_Handler();
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan){
	if(hcan == &hcan1){
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxDataBuffer);
		motor_receive();
	}
}


void CAN_SendMessage(uint16_t id,uint8_t len){
	txHeader.StdId = 0;
	txHeader.IDE = CAN_ID_EXT;
	txHeader.ExtId = id;
	txHeader.RTR = CAN_RTR_DATA;
	if(len>8){
		len = 8;
	}
	txHeader.DLC = len;
	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 0){
		HAL_CAN_AddTxMessage(&hcan1, &txHeader, txDataBuffer, &txMailBox);
	}
}

void motor_init(){
	motor_knee.device_id = 0x01;
	motor_ankle.device_id = 0x02;
	motor_knee.state = 0x01;
	motor_ankle.state = 0x01;
	can_set_pos_spd(motor_knee.device_id, 0,4,30000);
	can_set_pos_spd(motor_ankle.device_id,0,4,30000);
	HAL_Delay(2000);
	can_set_origin(motor_knee.device_id);
	can_set_origin(motor_ankle.device_id);
}

void motor_receive(){
	if(rxDataBuffer[7]!=0){
		motor_knee.state = 0x00;
		motor_ankle.state = 0x00;
		debugPrint("ERROR");
	}else{
		pos_int = (rxDataBuffer[0]<<8)|rxDataBuffer[1];
		vel_int = (rxDataBuffer[2]<<8)|rxDataBuffer[3];
		cur_int = (rxDataBuffer[4]<<8)|rxDataBuffer[5];
		temp_int = rxDataBuffer[6];
		if(rxHeader.StdId==motor_knee.device_id+0x2900){
			motor_knee.pos_actual = (float)(pos_int*0.1f);
			motor_knee.vel_actual = (float)(vel_int*10.0f);
			motor_knee.vel_actual = ERPM_TO_RADs(motor_knee.vel_actual);
			motor_knee.cur_actual = (float)(cur_int*0.01f);
			motor_knee.temperature = (float)temp_int;
		}else if (rxHeader.StdId==motor_ankle.device_id+0x2900){
			motor_ankle.pos_actual = (float)(pos_int*0.1f);
			motor_ankle.vel_actual = (float)(vel_int*10.0f);
			motor_ankle.vel_actual = ERPM_TO_RADs(motor_ankle.vel_actual);
			motor_ankle.cur_actual = (float)(cur_int*0.01f);
			motor_ankle.temperature = (float)temp_int;
		}
		Motor_UpdateMessages();
	}
}


void buffer_append_int32(uint8_t* buffer, int32_t number, uint8_t *index) {
	buffer[(*index)++] = number >> 24;
	buffer[(*index)++] = number >> 16;
	buffer[(*index)++] = number >> 8;
	buffer[(*index)++] = number;
}
void buffer_append_int16(uint8_t* buffer, int16_t number, uint8_t *index) {
	buffer[(*index)++] = number >> 8;
	buffer[(*index)++] = number;
}

void can_set_current(uint8_t controller_id, float current) {
	send_idx = 0;
	current = MAX(MIN(current,current_max),-current_max);
	buffer_append_int32(txDataBuffer, (int32_t)(current * 1000.0), &send_idx);
	msg_ext_id = controller_id|((uint32_t)CAN_PACKET_SET_CURRENT<<8);
	CAN_SendMessage(msg_ext_id,send_idx);
}

void can_set_pos_spd(uint8_t controller_id, float pos,float vel,float accel ){
	send_idx = 0;
	pos = MAX(MIN(pos, pos_max),-pos_max);
	buffer_append_int32(txDataBuffer, (int32_t)(pos * 10000.0), &send_idx);
	vel = MAX(MIN(vel,vel_max),-vel_max);
	vel = RADs_TO_ERPM(vel);
	buffer_append_int16(txDataBuffer,(int16_t)(vel/10.0), & send_idx);
	accel = MAX(MIN(accel,accel_max),0);
	buffer_append_int16(txDataBuffer,(int16_t)accel, & send_idx);
	msg_ext_id = controller_id|((uint32_t)CAN_PACKET_SET_POS_SPD<<8);
	CAN_SendMessage(msg_ext_id,send_idx);
}

void can_set_origin(uint8_t controller_id) {
	msg_ext_id = controller_id|((uint32_t)CAN_PACKET_SET_ORIGIN_HERE<<8);
	txDataBuffer[0] = 0x01;
	CAN_SendMessage(msg_ext_id,1);
}

void can_set_pos(uint8_t controller_id, float pos){
	pos = MAX(MIN(pos,pos_max),-pos_max);
	send_idx = 0;
	buffer_append_int32(txDataBuffer,(int32_t)(10000*pos),&send_idx);
	msg_ext_id = controller_id|((uint32_t)CAN_PACKET_SET_POS<<8);
	CAN_SendMessage(msg_ext_id,send_idx);
}
void can_set_vel(uint8_t controller_id, float vel){
	vel = MAX(MIN(vel,vel_max),-vel_max);
	vel = RADs_TO_ERPM(vel);
	buffer_append_int32(txDataBuffer,(int32_t)(vel), &send_idx);
	msg_ext_id = controller_id|((uint32_t)CAN_PACKET_SET_RPM<<8);
	CAN_SendMessage(msg_ext_id,send_idx);
}

void Motor_UpdateMessages(){
	m2p_motor.head = 0x01;
	uint16_t temp1 = 0;
	uint16_t temp2 = 0;
	temp1 = (uint16_t)(motor_knee.pos_actual*k_float2int12+b_float2int12);
	temp2 = (uint16_t)(motor_knee.vel_actual*k_float2int12+b_float2int12);
	m2p_motor.value1 = (uint16_t)(((temp1&0xfff)<<4)|(temp2>>8&0xf));

	temp1 = (uint16_t)(motor_ankle.pos_actual*k_float2int12+b_float2int12);
	m2p_motor.value2 = (uint16_t)(((temp2&0xff)<<8)|(temp1>>4&0xff));

	temp2 = (uint16_t)(motor_ankle.vel_actual*k_float2int12+b_float2int12);
	m2p_motor.value3 = (uint16_t)(((temp1&0xf)<<12)|(temp2&0xfff));

	temp1 = (uint16_t)(motor_knee.cur_actual*k_float2int12+b_float2int12);
	temp2 = (uint16_t)(motor_ankle.cur_actual*k_float2int12+b_float2int12);
	m2p_motor.value4 = (uint16_t)(((temp1&0xfff)<<4)|(temp2>>8&0xf));

	temp1 = (uint16_t)(motor_knee.temperature*k_float2int12+b_float2int12);
	m2p_motor.value5  =(uint16_t)(((temp2&0xff)<<8)|(temp1>>4&0xff));

	temp2 = (uint16_t)(motor_ankle.temperature*k_float2int12+b_float2int12);
	m2p_motor.value6 = (uint16_t)(((temp1&0xf)<<12)|(temp2&0xfff));
	m2p_motor.head = 0xFC;
}

void Motor_CMDUnpack(){
	uint16_t temp;
	if(p2m_motor.head==0xFC){
		if(p2m_motor.id==CMD_QUICK_STOP){
			motor_knee.state = 0x00;
			motor_ankle.state = 0x00;
			can_set_vel(motor_knee.device_id, 0);
			can_set_vel(motor_ankle.device_id, 0);
		}else if(p2m_motor.id==CMD_POSITION_CTRL){
			if(motor_knee.state==0x01&&motor_ankle.state==0x01){
				motor_knee.pos_desired = (float)((p2m_motor.value1-b_float2int16)/k_float2int16);
				can_set_pos(motor_knee.device_id, motor_knee.pos_desired);
				motor_ankle.pos_desired = (float)((p2m_motor.value2-b_float2int16)/k_float2int16);
				can_set_pos(motor_ankle.device_id, motor_ankle.pos_desired);
			}else{}
		}else if(p2m_motor.id==CMD_VELOCITY_CTRL){
			if(motor_knee.state==0x01&&motor_ankle.state==0x01){
				motor_knee.vel_desired = (float)((p2m_motor.value1-b_float2int16)/k_float2int16);
				can_set_vel(motor_knee.device_id, motor_knee.vel_desired);
				motor_ankle.vel_desired = (float)((p2m_motor.value2-b_float2int16)/k_float2int16);
				can_set_vel(motor_ankle.device_id, motor_ankle.vel_desired);
			}else{}
		}else if(p2m_motor.id==CMD_POSITION_AND_VELOCITY){
			if(motor_knee.state==0x01&&motor_ankle.state==0x01){
				motor_knee.pos_desired = (float)((p2m_motor.value1-b_float2int16)/k_float2int16);
				motor_knee.vel_desired = (float)((p2m_motor.value2-b_float2int16)/k_float2int16);
				motor_ankle.pos_desired = (float)((p2m_motor.value3-b_float2int16)/k_float2int16);
				motor_ankle.vel_desired = (float)((p2m_motor.value4-b_float2int16)/k_float2int16);
				can_set_pos_spd(motor_knee.device_id, motor_knee.pos_desired, motor_knee.vel_desired, 30000);
				can_set_pos_spd(motor_ankle.device_id, motor_ankle.pos_desired, motor_ankle.vel_desired, 30000);
			}else{}
		}else if(p2m_motor.id==CMD_TORQUE_CTRL){
			if(motor_knee.state==0x01&&motor_ankle.state==0x01){
				motor_knee.cur_desired = (float)((p2m_motor.value1-b_float2int16)/k_float2int16);
				can_set_current(motor_knee.device_id, motor_knee.cur_desired);
				motor_ankle.cur_desired = (float)((p2m_motor.value2-b_float2int16)/k_float2int16);
				can_set_current(motor_ankle.device_id, motor_ankle.cur_desired);
			}else{}
		}else if(p2m_motor.id==CMD_IMPEDANCE){
			if(motor_knee.state==0x01&&motor_ankle.state==0x01){
				temp = (uint16_t)(p2m_motor.value1>>4&0xfff);
				motor_knee.Kp =(float) (temp-b_float2int12)/k_float2int12;
				temp = (uint16_t)(((p2m_motor.value1&0xf)<<8)|(p2m_motor.value2>>8&0xff));
				motor_ankle.Kp = (float) (temp-b_float2int12)/k_float2int12;
				temp = (uint16_t)(((p2m_motor.value2&0xff)<<4)|(p2m_motor.value3>>12&0xf));
				motor_knee.Kb = (float) (temp-b_float2int12)/k_float2int12;
				temp = (uint16_t)(p2m_motor.value3&0xfff);
				motor_ankle.Kb = (float) (temp-b_float2int12)/k_float2int12;
				temp = (uint16_t)(p2m_motor.value4>>4&0xfff);
				motor_knee.Angle_eq = (float) (temp-b_float2int12)/k_float2int12;
				temp = (uint16_t)(((p2m_motor.value4&0xf)<<8)|(p2m_motor.ext_value));
				motor_ankle.Angle_eq = (float) (temp-b_float2int12)/k_float2int12;
				motor_knee.cur_desired = motor_knee.Kp*(motor_knee.pos_actual-motor_knee.Angle_eq)+motor_knee.Kb*motor_knee.vel_actual;
				motor_ankle.cur_desired = motor_ankle.Kp*(motor_ankle.pos_actual-motor_ankle.Angle_eq)+motor_ankle.Kb*motor_ankle.vel_actual;
				can_set_current(motor_knee.device_id, motor_knee.cur_desired);
				can_set_current(motor_ankle.device_id, motor_ankle.cur_desired);
			}else{}
		}else{}//end p2m_motor.id
	}else{}//end p2m_motor.head
}
void Motor_Debug_CMDUnpack(){
	uint16_t temp;
		if(p2m_motor.head==0xFC){
			if(p2m_motor.id==CMD_QUICK_STOP){
				motor_knee.state = 0x00;
				motor_ankle.state = 0x00;
				motor_knee.pos_actual = 0;
				motor_ankle.pos_actual = 0;
				motor_knee.vel_actual = 0;
				motor_ankle.vel_actual =0;
			}else if(p2m_motor.id==CMD_POSITION_CTRL){
				if(motor_knee.state==0x01&&motor_ankle.state==0x01){
					motor_knee.pos_desired = (float)((p2m_motor.value1-b_float2int16)/k_float2int16);
					motor_ankle.pos_desired = (float)((p2m_motor.value2-b_float2int16)/k_float2int16);
					motor_knee.pos_actual = motor_knee.pos_desired;
					motor_ankle.pos_actual = motor_ankle.pos_desired;
				}else{}
			}else if(p2m_motor.id==CMD_VELOCITY_CTRL){
				if(motor_knee.state==0x01&&motor_ankle.state==0x01){
					motor_knee.vel_desired = (float)((p2m_motor.value1-b_float2int16)/k_float2int16);
					motor_ankle.vel_desired = (float)((p2m_motor.value2-b_float2int16)/k_float2int16);
					motor_knee.vel_actual = motor_knee.vel_desired;
					motor_ankle.vel_actual = motor_ankle.vel_desired;
				}else{}
			}else if(p2m_motor.id==CMD_POSITION_AND_VELOCITY){
				if(motor_knee.state==0x01&&motor_ankle.state==0x01){
					motor_knee.pos_desired = (float)((p2m_motor.value1-b_float2int16)/k_float2int16);
					motor_knee.vel_desired = (float)((p2m_motor.value2-b_float2int16)/k_float2int16);
					motor_ankle.pos_desired = (float)((p2m_motor.value3-b_float2int16)/k_float2int16);
					motor_ankle.vel_desired = (float)((p2m_motor.value4-b_float2int16)/k_float2int16);
					motor_knee.pos_actual = motor_knee.pos_desired;
					motor_ankle.pos_actual = motor_ankle.pos_desired;
					motor_knee.vel_actual = motor_knee.vel_desired;
					motor_ankle.vel_actual = motor_ankle.vel_desired;
				}else{}
			}else if(p2m_motor.id==CMD_TORQUE_CTRL){
				if(motor_knee.state==0x01&&motor_ankle.state==0x01){
					motor_knee.cur_desired = (float)((p2m_motor.value1-b_float2int16)/k_float2int16);
					motor_ankle.cur_desired = (float)((p2m_motor.value2-b_float2int16)/k_float2int16);
					motor_knee.cur_actual = motor_knee.cur_desired;
					motor_ankle.cur_actual = motor_ankle.cur_desired;
				}else{}
			}else if(p2m_motor.id==CMD_IMPEDANCE){
				if(motor_knee.state==0x01&&motor_ankle.state==0x01){
					temp = (uint16_t)(p2m_motor.value1>>4&0xfff);
					motor_knee.Kp =(float) (temp-b_float2int12)/k_float2int12;
					temp = (uint16_t)((p2m_motor.value1&0xf<<8)|(p2m_motor.value2>>8&0xff));
					motor_ankle.Kp = (float) (temp-b_float2int12)/k_float2int12;
					temp = (uint16_t)((p2m_motor.value2&0xff<<4)|(p2m_motor.value3>>12&0xf));
					motor_knee.Kb = (float) (temp-b_float2int12)/k_float2int12;
					temp = (uint16_t)(p2m_motor.value3&0xfff);
					motor_ankle.Kb = (float) (temp-b_float2int12)/k_float2int12;
					temp = (uint16_t)(p2m_motor.value4>>4&0xfff);
					motor_knee.Angle_eq = (float) (temp-b_float2int12)/k_float2int12;
					temp = (uint16_t)((p2m_motor.value4&0xf<<8)|(p2m_motor.ext_value));
					motor_ankle.Angle_eq = (float) (temp-b_float2int12)/k_float2int12;
					motor_knee.cur_desired = motor_knee.Kp*(motor_knee.pos_actual-motor_knee.Angle_eq)+motor_knee.Kb*motor_knee.vel_actual;
					motor_ankle.cur_desired = motor_ankle.Kp*(motor_ankle.pos_actual-motor_ankle.Angle_eq)+motor_ankle.Kb*motor_ankle.vel_actual;
					motor_knee.cur_actual = motor_knee.cur_desired;
					motor_ankle.cur_actual = motor_ankle.cur_desired;
				}else{}
			}else{}//end p2m_motor.id
		}else{}//end p2m_motor.head
		Motor_UpdateMessages();
}
