################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include User/BSP/Src/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
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
LM25118_PowerModule.hex \

SECONDARY_LIST += \
LM25118_PowerModule.lst \

SECONDARY_SIZE += \
LM25118_PowerModule.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: LM25118_PowerModule.elf secondary-outputs

# Tool invocations
LM25118_PowerModule.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -T "c:/Users/Kun/Desktop/LM25118_PowerModule/App-CH32V003/Ld/Link.ld" -nostartfiles -Xlinker --gc-sections -Xlinker --print-memory-usage -Wl,-Map,"LM25118_PowerModule.map" --specs=nano.specs --specs=nosys.specs -o "LM25118_PowerModule.elf" $(OBJS) $(LIBS) -lprintf
LM25118_PowerModule.hex: LM25118_PowerModule.elf
	@	riscv-none-embed-objcopy -O ihex "LM25118_PowerModule.elf" "LM25118_PowerModule.hex"
LM25118_PowerModule.lst: LM25118_PowerModule.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "LM25118_PowerModule.elf" > "LM25118_PowerModule.lst"
LM25118_PowerModule.siz: LM25118_PowerModule.elf
	riscv-none-embed-size --format=berkeley "LM25118_PowerModule.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) LM25118_PowerModule.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets