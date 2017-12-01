################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/circular_buffer.c \
../src/eeprom.c \
../src/gpio.c \
../src/i2c_master.c \
../src/keypad.c \
../src/main.c \
../src/oled96.c \
../src/print.c \
../src/rcc.c \
../src/rtc.c \
../src/setup.c \
../src/sht10.c \
../src/spi.c \
../src/syscalls.c \
../src/system_stm32l0xx.c \
../src/systick.c \
../src/timer.c \
../src/widgets.c 

OBJS += \
./src/circular_buffer.o \
./src/eeprom.o \
./src/gpio.o \
./src/i2c_master.o \
./src/keypad.o \
./src/main.o \
./src/oled96.o \
./src/print.o \
./src/rcc.o \
./src/rtc.o \
./src/setup.o \
./src/sht10.o \
./src/spi.o \
./src/syscalls.o \
./src/system_stm32l0xx.o \
./src/systick.o \
./src/timer.o \
./src/widgets.o 

C_DEPS += \
./src/circular_buffer.d \
./src/eeprom.d \
./src/gpio.d \
./src/i2c_master.d \
./src/keypad.d \
./src/main.d \
./src/oled96.d \
./src/print.d \
./src/rcc.d \
./src/rtc.d \
./src/setup.d \
./src/sht10.d \
./src/spi.d \
./src/syscalls.d \
./src/system_stm32l0xx.d \
./src/systick.d \
./src/timer.d \
./src/widgets.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft -DSTM32L0 -DNUCLEO_L031K6 -DSTM32 -DSTM32L031K6Tx -DDEBUG -DSTM32L031xx -I"/home/mark/repositories/shroom-puter-2/stm32_workspace/climate_control/inc" -I"/home/mark/repositories/shroom-puter-2/stm32_workspace/climate_control/CMSIS/core" -I"/home/mark/repositories/shroom-puter-2/stm32_workspace/climate_control/CMSIS/device" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


