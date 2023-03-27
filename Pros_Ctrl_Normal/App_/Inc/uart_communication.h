#ifndef INC_UART_COMMUNICATION_H_
#define INC_UART_COMMUNICATION_H_
typedef enum{
	CMD_POSITION_CTRL,
	CMD_VELOCITY_CTRL,
	CMD_TORQUE_CTRL,
	CMD_POSITION_AND_VELOCITY,
	CMD_IMPEDANCE,
	CMD_QUICK_STOP
}CMD_PACKET_ID;

void Start_PCReceiveIT();
void PC_UnpackMessage();
void PC_PackMessage();
void PC_CommunicationErrorHandler();
#endif /* INC_UART_COMMUNICATION_H_ */
