################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Code/Third-Party_lib/Debug_Protocol/Debug_Protocol.c \
../Code/Third-Party_lib/Debug_Protocol/PerformanceTest.c 

OBJS += \
./Code/Third-Party_lib/Debug_Protocol/Debug_Protocol.o \
./Code/Third-Party_lib/Debug_Protocol/PerformanceTest.o 

C_DEPS += \
./Code/Third-Party_lib/Debug_Protocol/Debug_Protocol.d \
./Code/Third-Party_lib/Debug_Protocol/PerformanceTest.d 


# Each subdirectory must supply rules for building sources it contributes
Code/Third-Party_lib/Debug_Protocol/%.o Code/Third-Party_lib/Debug_Protocol/%.su Code/Third-Party_lib/Debug_Protocol/%.cyclo: ../Code/Third-Party_lib/Debug_Protocol/%.c Code/Third-Party_lib/Debug_Protocol/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/RyanY/STM32CubeIDE/workspace_1.18.0/H7Demo/Code/User" -I"C:/Users/RyanY/STM32CubeIDE/workspace_1.18.0/H7Demo/Code/Third-Party_lib/Debug_Protocol" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Code-2f-Third-2d-Party_lib-2f-Debug_Protocol

clean-Code-2f-Third-2d-Party_lib-2f-Debug_Protocol:
	-$(RM) ./Code/Third-Party_lib/Debug_Protocol/Debug_Protocol.cyclo ./Code/Third-Party_lib/Debug_Protocol/Debug_Protocol.d ./Code/Third-Party_lib/Debug_Protocol/Debug_Protocol.o ./Code/Third-Party_lib/Debug_Protocol/Debug_Protocol.su ./Code/Third-Party_lib/Debug_Protocol/PerformanceTest.cyclo ./Code/Third-Party_lib/Debug_Protocol/PerformanceTest.d ./Code/Third-Party_lib/Debug_Protocol/PerformanceTest.o ./Code/Third-Party_lib/Debug_Protocol/PerformanceTest.su

.PHONY: clean-Code-2f-Third-2d-Party_lib-2f-Debug_Protocol

