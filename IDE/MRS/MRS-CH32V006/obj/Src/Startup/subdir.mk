################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup/startup_ch32v00x.S 

S_UPPER_DEPS += \
./Src/Startup/startup_ch32v00x.d 

OBJS += \
./Src/Startup/startup_ch32v00x.o 

DIR_OBJS += \
./Src/Startup/*.o \

DIR_DEPS += \
./Src/Startup/*.d \

DIR_EXPANDS += \
./Src/Startup/*.253r.expand \


# Each subdirectory must supply rules for building sources it contributes
Src/Startup/startup_ch32v00x.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup/startup_ch32v00x.S
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

