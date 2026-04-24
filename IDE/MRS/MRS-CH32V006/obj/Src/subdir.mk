################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/ch32v00X_it.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/main.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/system_ch32v00X.c 

C_DEPS += \
./Src/ch32v00X_it.d \
./Src/main.d \
./Src/system_ch32v00X.d 

OBJS += \
./Src/ch32v00X_it.o \
./Src/main.o \
./Src/system_ch32v00X.o 

DIR_OBJS += \
./Src/*.o \

DIR_DEPS += \
./Src/*.d \

DIR_EXPANDS += \
./Src/*.253r.expand \


# Each subdirectory must supply rules for building sources it contributes
Src/ch32v00X_it.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/ch32v00X_it.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/main.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/main.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/system_ch32v00X.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/system_ch32v00X.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

