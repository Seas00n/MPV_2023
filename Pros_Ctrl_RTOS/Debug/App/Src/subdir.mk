################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/debug_uart.c \
../App/Src/motor.c \
../App/Src/msg.c \
../App/Src/uart_communication.c 

OBJS += \
./App/Src/debug_uart.o \
./App/Src/motor.o \
./App/Src/msg.o \
./App/Src/uart_communication.o 

C_DEPS += \
./App/Src/debug_uart.d \
./App/Src/motor.d \
./App/Src/msg.d \
./App/Src/uart_communication.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/%.o App/Src/%.su: ../App/Src/%.c App/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F427xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"/home/yuxuan/Project/MPV_2023/Pros_Ctrl_RTOS/App/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Src

clean-App-2f-Src:
	-$(RM) ./App/Src/debug_uart.d ./App/Src/debug_uart.o ./App/Src/debug_uart.su ./App/Src/motor.d ./App/Src/motor.o ./App/Src/motor.su ./App/Src/msg.d ./App/Src/msg.o ./App/Src/msg.su ./App/Src/uart_communication.d ./App/Src/uart_communication.o ./App/Src/uart_communication.su

.PHONY: clean-App-2f-Src

