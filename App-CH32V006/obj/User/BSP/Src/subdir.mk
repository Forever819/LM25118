################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/BSP/Src/ADC_k.c \
../User/BSP/Src/Buzzer_k.c \
../User/BSP/Src/Encoder_k.c \
../User/BSP/Src/Key_k.c \
../User/BSP/Src/OLED.c \
../User/BSP/Src/OLED_Data.c \
../User/BSP/Src/PID.c \
../User/BSP/Src/Timer_k.c \
../User/BSP/Src/WS2812_k.c 

C_DEPS += \
./User/BSP/Src/ADC_k.d \
./User/BSP/Src/Buzzer_k.d \
./User/BSP/Src/Encoder_k.d \
./User/BSP/Src/Key_k.d \
./User/BSP/Src/OLED.d \
./User/BSP/Src/OLED_Data.d \
./User/BSP/Src/PID.d \
./User/BSP/Src/Timer_k.d \
./User/BSP/Src/WS2812_k.d 

OBJS += \
./User/BSP/Src/ADC_k.o \
./User/BSP/Src/Buzzer_k.o \
./User/BSP/Src/Encoder_k.o \
./User/BSP/Src/Key_k.o \
./User/BSP/Src/OLED.o \
./User/BSP/Src/OLED_Data.o \
./User/BSP/Src/PID.o \
./User/BSP/Src/Timer_k.o \
./User/BSP/Src/WS2812_k.o 

DIR_OBJS += \
./User/BSP/Src/*.o \

DIR_DEPS += \
./User/BSP/Src/*.d \

DIR_EXPANDS += \
./User/BSP/Src/*.253r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/BSP/Src/%.o: ../User/BSP/Src/%.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"c:/Users/Kun/Desktop/LM25118_PowerModule/App-CH32V006/Debug" -I"c:/Users/Kun/Desktop/LM25118_PowerModule/App-CH32V006/Core" -I"c:/Users/Kun/Desktop/LM25118_PowerModule/App-CH32V006/User" -I"c:/Users/Kun/Desktop/LM25118_PowerModule/App-CH32V006/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118_PowerModule/App-CH32V006/User/BSP/Inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

