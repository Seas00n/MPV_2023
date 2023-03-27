################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App_/Src/debug_uart.c \
../App_/Src/motor.c \
../App_/Src/msg.c \
../App_/Src/uart_communication.c 

OBJS += \
./App_/Src/debug_uart.o \
./App_/Src/motor.o \
./App_/Src/msg.o \
./App_/Src/uart_communication.o 

C_DEPS += \
./App_/Src/debug_uart.d \
./App_/Src/motor.d \
./App_/Src/msg.d \
./App_/Src/uart_communication.d 


# Each subdirectory must supply rules for building sources it contributes
App_/Src/%.o App_/Src/%.su: ../App_/Src/%.c App_/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I/home/yuxuan/Project/MPV_2023/Pros_Ctrl_Normal/App_/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App_-2f-Src

clean-App_-2f-Src:
	-$(RM) ./App_/Src/debug_uart.d ./App_/Src/debug_uart.o ./App_/Src/debug_uart.su ./App_/Src/motor.d ./App_/Src/motor.o ./App_/Src/motor.su ./App_/Src/msg.d ./App_/Src/msg.o ./App_/Src/msg.su ./App_/Src/uart_communication.d ./App_/Src/uart_communication.o ./App_/Src/uart_communication.su

.PHONY: clean-App_-2f-Src

