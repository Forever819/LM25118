# EIDE Build Skill — 自动生成的 RISC-V GCC 工具链文件
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(GCC_BIN "C:/Users/Kun/.eide/tools/gcc_riscv/bin")
set(GCC_PREFIX "riscv-none-embed-")

set(CMAKE_C_COMPILER   "${GCC_BIN}/${GCC_PREFIX}gcc.exe")
set(CMAKE_CXX_COMPILER "${GCC_BIN}/${GCC_PREFIX}g++.exe")
set(CMAKE_ASM_COMPILER "${GCC_BIN}/${GCC_PREFIX}gcc.exe")

set(CMAKE_C_FLAGS "-march=rv32ec -mabi=ilp32e -mcmodel=medlow -ffunction-sections -fdata-sections -fno-common -Os -g -std=gnu99 -Wall -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized --specs=nano.specs --specs=nosys.specs -Wl,-Bstatic" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "-march=rv32ec -mabi=ilp32e -mcmodel=medlow -x assembler-with-cpp -g -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized --specs=nano.specs --specs=nosys.specs -Wl,-Bstatic" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "" FORCE)

set(CMAKE_C_LINK_EXECUTABLE
    "<CMAKE_C_COMPILER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
