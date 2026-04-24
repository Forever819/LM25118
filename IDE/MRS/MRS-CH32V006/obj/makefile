################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Src/Startup/subdir.mk
-include Src/Peripheral/src/subdir.mk
-include Src/Debug/subdir.mk
-include Src/Core/subdir.mk
-include Src/BSP/Src/subdir.mk
-include Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
LM25118数控电源.hex \

SECONDARY_LIST += \
LM25118数控电源.lst \

SECONDARY_SIZE += \
LM25118数控电源.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: LM25118数控电源.elf secondary-outputs

# Tool invocations
LM25118数控电源.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-wch-elf-gcc -march=rv32ec_zmmul_xw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "c:\Users\Kun\Desktop\LM25118\LM25118_PowerModule\Src\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"LM25118数控电源.map" --specs=nano.specs --specs=nosys.specs -o "LM25118数控电源.elf" $(OBJS) $(LIBS) -lprintfloat
LM25118数控电源.hex: LM25118数控电源.elf
	@	riscv-wch-elf-objcopy -O ihex "LM25118数控电源.elf" "LM25118数控电源.hex"
LM25118数控电源.lst: LM25118数控电源.elf
	@	riscv-wch-elf-objdump --all-headers --demangle --disassemble -M xw "LM25118数控电源.elf" > "LM25118数控电源.lst"
LM25118数控电源.siz: LM25118数控电源.elf
	riscv-wch-elf-size --format=berkeley "LM25118数控电源.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) LM25118数控电源.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets