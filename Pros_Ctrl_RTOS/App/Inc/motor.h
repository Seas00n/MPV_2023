/*
 * motor.h
 *
 *  Created on: Feb 22, 2023
 *      Author: yuxuan
 */


#ifndef INC_MOTOR_
#define INC_MOTOR_
typedef struct{
	uint16_t device_id;
	float pos_actual;
	float vel_actual;
	float cur_actual;
	float pos_desired;
	float vel_desired;
	float cur_desired;
	float Kp;
	float Kb;
	float Angle_eq;
	float temperature;
	uint8_t state;
	uint8_t is_free;
}MotorTypeDef;


void CAN_FilterConfig();
void CAN_SendMessage(uint16_t id, uint8_t length);
void buffer_append_int32(uint8_t* buffer, int32_t number, uint8_t *index);
void buffer_append_int16(uint8_t* buffer, int16_t number, uint8_t *index);
void can_set_current(uint8_t controller_id, float current);
void can_set_pos_spd(uint8_t controller_id, float pos,float vel,float accel );
void can_set_pos(uint8_t controller_id, float pos);
void can_set_vel(uint8_t controller_id, float vel);
void can_set_origin(uint8_t controller_id);
void motor_receive();
void motor_init();
void Motor_UpdateMessages();
void Motor_CMDUnpack();
void Motor_Debug_CMDUnpack();
#endif /* INC_UART_COMMUNICATION_H_ */
