# Makefile for CH32V006 RISC-V MCU (auto-generated from EIDE builder.params)
# Toolchain: MounRiver RISC-V Embedded GCC

TC_ROOT  := c:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC
TC_BIN   := $(TC_ROOT)/bin
PREFIX   := riscv-none-embed-
CC       := "$(TC_BIN)/$(PREFIX)gcc.exe"
OBJCOPY  := "$(TC_BIN)/$(PREFIX)objcopy.exe"
SZ       := "$(TC_BIN)/$(PREFIX)size.exe"

# Proj root relative to this Makefile
PROJ     := ../..

ARCH     := rv32ec
ABI      := ilp32e
CMODEL   := medlow

INC      := -I../../Src/Core -I../../Src/Debug -I../../Src/Peripheral/inc -I../../Src -I../../Src/BSP/inc

CFLAGS   := -c -std=gnu99 $(INC) -march=$(ARCH) -mcmodel=$(CMODEL) -mabi=$(ABI) \
            -Os -funsigned-char -Wall -g -ffunction-sections -fdata-sections \
            -fmessage-length=0 -fsigned-char -Wunused -Wuninitialized \
            --specs=nano.specs --specs=nosys.specs -Wl,-Bstatic

ASMFLAGS := -c -x assembler-with-cpp $(INC) -march=$(ARCH) -mcmodel=$(CMODEL) -mabi=$(ABI) \
            -g -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections \
            -Wunused -Wuninitialized --specs=nano.specs --specs=nosys.specs -Wl,-Bstatic

LDFLAGS  := -march=$(ARCH) -mcmodel=$(CMODEL) -mabi=$(ABI) \
            -Wl,--gc-sections -nostartfiles -T"../../Src/Ld/Link.ld" \
            --specs=nano.specs --specs=nosys.specs -Wl,-Bstatic \
            -Wl,-u,_printf_float -lprintfloat -lm

OUTDIR   := build/obj
TARGET   := $(OUTDIR)/CH32V006

# Source files (relative to IDE/EIDE)
SRCS_C   := ../../Src/BSP/Src/ADC.c \
            ../../Src/BSP/Src/Buzzer.c \
            ../../Src/BSP/Src/Encoder.c \
            ../../Src/BSP/Src/OELD_UI.c \
            ../../Src/BSP/Src/OLED.c \
            ../../Src/BSP/Src/OLED_Data.c \
            ../../Src/BSP/Src/PID.c \
            ../../Src/BSP/Src/Timer.c \
            ../../Src/BSP/Src/WS2812.c \
            ../../Src/BSP/Src/bm_img.c \
            ../../Src/BSP/Src/flash_param.c \
            ../../Src/Core/core_riscv.c \
            ../../Src/Debug/debug.c \
            ../../Src/Peripheral/src/ch32v00x_adc.c \
            ../../Src/Peripheral/src/ch32v00x_dbgmcu.c \
            ../../Src/Peripheral/src/ch32v00x_dma.c \
            ../../Src/Peripheral/src/ch32v00x_exti.c \
            ../../Src/Peripheral/src/ch32v00x_flash.c \
            ../../Src/Peripheral/src/ch32v00x_gpio.c \
            ../../Src/Peripheral/src/ch32v00x_i2c.c \
            ../../Src/Peripheral/src/ch32v00x_iwdg.c \
            ../../Src/Peripheral/src/ch32v00x_misc.c \
            ../../Src/Peripheral/src/ch32v00x_opa.c \
            ../../Src/Peripheral/src/ch32v00x_pwr.c \
            ../../Src/Peripheral/src/ch32v00x_rcc.c \
            ../../Src/Peripheral/src/ch32v00x_spi.c \
            ../../Src/Peripheral/src/ch32v00x_tim.c \
            ../../Src/Peripheral/src/ch32v00x_usart.c \
            ../../Src/Peripheral/src/ch32v00x_wwdg.c \
            ../../Src/ch32v00X_it.c \
            ../../Src/Event_Bus.c \
            ../../Src/Digital_Power_Core.c \
            ../../Src/Responders/UI_Responder.c \
            ../../Src/Responders/LED_Responder.c \
            ../../Src/Responders/Buzzer_Responder.c \
            ../../Src/main.c \
            ../../Src/system_ch32v00X.c

SRCS_ASM := ../../Src/Startup/startup_ch32v00x.S

OBJS_C   := $(patsubst ../../%.c,$(OUTDIR)/%.o,$(SRCS_C))
OBJS_ASM := $(patsubst ../../%.S,$(OUTDIR)/%.o,$(SRCS_ASM))
OBJS     := $(OBJS_C) $(OBJS_ASM)

ELF := $(TARGET).elf
HEX := $(TARGET).hex
BIN := $(TARGET).bin

.PHONY: all clean

all: $(ELF) $(HEX) $(BIN)
	@echo "============================================"
	@echo "  Build complete: CH32V006"
	@echo "============================================"
	@$(SZ) $(ELF)
	@echo ""

# Compile C
$(OUTDIR)/%.o: ../../%.c
	@mkdir -p "$(dir $@)"
	@echo "  CC     $<"
	@$(CC) $(CFLAGS) -o "$@" "$<"

# Assemble
$(OUTDIR)/%.o: ../../%.S
	@mkdir -p "$(dir $@)"
	@echo "  ASM   $<"
	@$(CC) $(ASMFLAGS) -o "$@" "$<"

# Link
$(ELF): $(OBJS)
	@echo "  LD    $(notdir $@)"
	@$(CC) -march=$(ARCH) -mcmodel=$(CMODEL) -mabi=$(ABI) \
		-Wl,--gc-sections -nostartfiles -T"../../Src/Ld/Link.ld" \
		--specs=nano.specs --specs=nosys.specs -Wl,-Bstatic \
		$(OBJS) -o "$@" -Wl,-u,_printf_float -lprintfloat -lm

# Hex
$(HEX): $(ELF)
	@echo "  HEX   $(notdir $@)"
	@$(OBJCOPY) -O ihex "$<" "$@"

# Bin
$(BIN): $(ELF)
	@echo "  BIN   $(notdir $@)"
	@$(OBJCOPY) -O binary "$<" "$@"

clean:
	rm -rf $(OUTDIR)
