################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug/debug.c 

C_DEPS += \
./Src/Debug/debug.d 

OBJS += \
./Src/Debug/debug.o 

DIR_OBJS += \
./Src/Debug/*.o \

DIR_DEPS += \
./Src/Debug/*.d \

DIR_EXPANDS += \
./Src/Debug/*.253r.expand \


# Each subdirectory must supply rules for building sources it contributes
Src/Debug/debug.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug/debug.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

