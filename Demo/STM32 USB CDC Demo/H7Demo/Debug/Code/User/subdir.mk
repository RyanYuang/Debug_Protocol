################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Code/User/User_Code.c 

OBJS += \
./Code/User/User_Code.o 

C_DEPS += \
./Code/User/User_Code.d 


# Each subdirectory must supply rules for building sources it contributes
Code/User/%.o Code/User/%.su Code/User/%.cyclo: ../Code/User/%.c Code/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/RyanY/STM32CubeIDE/workspace_1.18.0/H7Demo/Code/User" -I"C:/Users/RyanY/STM32CubeIDE/workspace_1.18.0/H7Demo/Code/Third-Party_lib/Debug_Protocol" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Code-2f-User

clean-Code-2f-User:
	-$(RM) ./Code/User/User_Code.cyclo ./Code/User/User_Code.d ./Code/User/User_Code.o ./Code/User/User_Code.su

.PHONY: clean-Code-2f-User

