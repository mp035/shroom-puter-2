################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/circular_buffer.c \
../src/eeprom.c \
../src/fonts.c \
../src/gpio.c \
../src/i2c_master.c \
../src/main.c \
../src/menu.c \
../src/oled96.c \
../src/print.c \
../src/rcc.c \
../src/rotary_encoder.c \
../src/rtc.c \
../src/setup.c \
../src/sht10.c \
../src/syscalls.c \
../src/system_stm32l0xx.c \
../src/systick.c \
../src/timer.c \
../src/uart2.c \
../src/widgets.c 

OBJS += \
./src/circular_buffer.o \
./src/eeprom.o \
./src/fonts.o \
./src/gpio.o \
./src/i2c_master.o \
./src/main.o \
./src/menu.o \
./src/oled96.o \
./src/print.o \
./src/rcc.o \
./src/rotary_encoder.o \
./src/rtc.o \
./src/setup.o \
./src/sht10.o \
./src/syscalls.o \
./src/system_stm32l0xx.o \
./src/systick.o \
./src/timer.o \
./src/uart2.o \
./src/widgets.o 

C_DEPS += \
./src/circular_buffer.d \
./src/eeprom.d \
./src/fonts.d \
./src/gpio.d \
./src/i2c_master.d \
./src/main.d \
./src/menu.d \
./src/oled96.d \
./src/print.d \
./src/rcc.d \
./src/rotary_encoder.d \
./src/rtc.d \
./src/setup.d \
./src/sht10.d \
./src/syscalls.d \
./src/system_stm32l0xx.d \
./src/systick.d \
./src/timer.d \
./src/uart2.d \
./src/widgets.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft -DSTM32L0 -DNUCLEO_L031K6 -DSTM32 -DSTM32L031K6Tx -DSTM32L031xx -I"/home/mark/repositories/shroom-puter-2/sw4stm32_workspace/climate_control/inc" -I"/home/mark/repositories/shroom-puter-2/sw4stm32_workspace/climate_control/CMSIS/core" -I"/home/mark/repositories/shroom-puter-2/sw4stm32_workspace/climate_control/CMSIS/device" -I"/home/mark/repositories/shroom-puter-2/sw4stm32_workspace/climate_control/Utilities" -O3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


