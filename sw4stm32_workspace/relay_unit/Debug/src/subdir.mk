################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/circular_buffer.c \
../src/gpio.c \
../src/main.c \
../src/rcc.c \
../src/syscalls.c \
../src/system_stm32l0xx.c \
../src/systick.c \
../src/uart2.c 

OBJS += \
./src/circular_buffer.o \
./src/gpio.o \
./src/main.o \
./src/rcc.o \
./src/syscalls.o \
./src/system_stm32l0xx.o \
./src/systick.o \
./src/uart2.o 

C_DEPS += \
./src/circular_buffer.d \
./src/gpio.d \
./src/main.d \
./src/rcc.d \
./src/syscalls.d \
./src/system_stm32l0xx.d \
./src/systick.d \
./src/uart2.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft -DSTM32 -DSTM32L0 -DSTM32L031K6Tx -DNUCLEO_L031K6 -DDEBUG -DSTM32L031xx -I"/home/mark/temp/sw4stm32_workspace/relay_unit/Utilities" -I"/home/mark/temp/sw4stm32_workspace/relay_unit/inc" -I"/home/mark/temp/sw4stm32_workspace/relay_unit/CMSIS/device" -I"/home/mark/temp/sw4stm32_workspace/relay_unit/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


