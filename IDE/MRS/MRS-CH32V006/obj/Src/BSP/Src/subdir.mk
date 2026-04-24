################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/ADC_k.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/Buzzer_k.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/Encoder_k.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/OELD_UI.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/OLED.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/OLED_Data.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/PID.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/Timer_k.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/WS2812_k.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/bm_img.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/flash_param.c 

C_DEPS += \
./Src/BSP/Src/ADC_k.d \
./Src/BSP/Src/Buzzer_k.d \
./Src/BSP/Src/Encoder_k.d \
./Src/BSP/Src/OELD_UI.d \
./Src/BSP/Src/OLED.d \
./Src/BSP/Src/OLED_Data.d \
./Src/BSP/Src/PID.d \
./Src/BSP/Src/Timer_k.d \
./Src/BSP/Src/WS2812_k.d \
./Src/BSP/Src/bm_img.d \
./Src/BSP/Src/flash_param.d 

OBJS += \
./Src/BSP/Src/ADC_k.o \
./Src/BSP/Src/Buzzer_k.o \
./Src/BSP/Src/Encoder_k.o \
./Src/BSP/Src/OELD_UI.o \
./Src/BSP/Src/OLED.o \
./Src/BSP/Src/OLED_Data.o \
./Src/BSP/Src/PID.o \
./Src/BSP/Src/Timer_k.o \
./Src/BSP/Src/WS2812_k.o \
./Src/BSP/Src/bm_img.o \
./Src/BSP/Src/flash_param.o 

DIR_OBJS += \
./Src/BSP/Src/*.o \

DIR_DEPS += \
./Src/BSP/Src/*.d \

DIR_EXPANDS += \
./Src/BSP/Src/*.253r.expand \


# Each subdirectory must supply rules for building sources it contributes
Src/BSP/Src/ADC_k.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/ADC_k.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/Buzzer_k.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/Buzzer_k.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/Encoder_k.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/Encoder_k.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/OELD_UI.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/OELD_UI.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/OLED.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/OLED.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/OLED_Data.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/OLED_Data.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/PID.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/PID.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/Timer_k.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/Timer_k.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/WS2812_k.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/WS2812_k.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/bm_img.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/bm_img.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/BSP/Src/flash_param.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Src/flash_param.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

