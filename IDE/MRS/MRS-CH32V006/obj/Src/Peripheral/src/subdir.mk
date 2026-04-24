################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_adc.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_dbgmcu.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_dma.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_exti.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_flash.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_gpio.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_i2c.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_iwdg.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_misc.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_opa.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_pwr.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_rcc.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_spi.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_tim.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_usart.c \
c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_wwdg.c 

C_DEPS += \
./Src/Peripheral/src/ch32v00x_adc.d \
./Src/Peripheral/src/ch32v00x_dbgmcu.d \
./Src/Peripheral/src/ch32v00x_dma.d \
./Src/Peripheral/src/ch32v00x_exti.d \
./Src/Peripheral/src/ch32v00x_flash.d \
./Src/Peripheral/src/ch32v00x_gpio.d \
./Src/Peripheral/src/ch32v00x_i2c.d \
./Src/Peripheral/src/ch32v00x_iwdg.d \
./Src/Peripheral/src/ch32v00x_misc.d \
./Src/Peripheral/src/ch32v00x_opa.d \
./Src/Peripheral/src/ch32v00x_pwr.d \
./Src/Peripheral/src/ch32v00x_rcc.d \
./Src/Peripheral/src/ch32v00x_spi.d \
./Src/Peripheral/src/ch32v00x_tim.d \
./Src/Peripheral/src/ch32v00x_usart.d \
./Src/Peripheral/src/ch32v00x_wwdg.d 

OBJS += \
./Src/Peripheral/src/ch32v00x_adc.o \
./Src/Peripheral/src/ch32v00x_dbgmcu.o \
./Src/Peripheral/src/ch32v00x_dma.o \
./Src/Peripheral/src/ch32v00x_exti.o \
./Src/Peripheral/src/ch32v00x_flash.o \
./Src/Peripheral/src/ch32v00x_gpio.o \
./Src/Peripheral/src/ch32v00x_i2c.o \
./Src/Peripheral/src/ch32v00x_iwdg.o \
./Src/Peripheral/src/ch32v00x_misc.o \
./Src/Peripheral/src/ch32v00x_opa.o \
./Src/Peripheral/src/ch32v00x_pwr.o \
./Src/Peripheral/src/ch32v00x_rcc.o \
./Src/Peripheral/src/ch32v00x_spi.o \
./Src/Peripheral/src/ch32v00x_tim.o \
./Src/Peripheral/src/ch32v00x_usart.o \
./Src/Peripheral/src/ch32v00x_wwdg.o 

DIR_OBJS += \
./Src/Peripheral/src/*.o \

DIR_DEPS += \
./Src/Peripheral/src/*.d \

DIR_EXPANDS += \
./Src/Peripheral/src/*.253r.expand \


# Each subdirectory must supply rules for building sources it contributes
Src/Peripheral/src/ch32v00x_adc.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_adc.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_dbgmcu.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_dbgmcu.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_dma.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_dma.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_exti.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_exti.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_flash.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_flash.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_gpio.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_gpio.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_i2c.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_i2c.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_iwdg.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_iwdg.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_misc.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_misc.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_opa.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_opa.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_pwr.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_pwr.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_rcc.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_rcc.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_spi.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_spi.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_tim.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_tim.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_usart.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_usart.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
Src/Peripheral/src/ch32v00x_wwdg.o: c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/src/ch32v00x_wwdg.c
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"C:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/BSP/Inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Core" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Debug" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Ld" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Peripheral/inc" -I"c:/Users/Kun/Desktop/LM25118/LM25118_PowerModule/Src/Startup" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

