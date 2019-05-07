################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/7seg.c \
../src/EXTI.c \
../src/Encoder.c \
../src/PWM.c \
../src/led.c \
../src/main.c \
../src/motor.c 

OBJS += \
./src/7seg.o \
./src/EXTI.o \
./src/Encoder.o \
./src/PWM.o \
./src/led.o \
./src/main.o \
./src/motor.o 

C_DEPS += \
./src/7seg.d \
./src/EXTI.d \
./src/Encoder.d \
./src/PWM.d \
./src/led.d \
./src/main.d \
./src/motor.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DSTM32L4 -DSTM32L476RGTx -DNUCLEO_L476RG -DDEBUG -I"C:/Users/jimmy/workspace/DHT11/inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


