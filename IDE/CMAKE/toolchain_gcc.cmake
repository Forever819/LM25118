# GCC (riscv-none-embed) CMake toolchain ¡ª auto-generated
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(GCC_BIN "C:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin")
set(GCC_PREFIX "riscv-none-embed-")

set(CMAKE_C_COMPILER   "${GCC_BIN}/${GCC_PREFIX}gcc.exe")
set(CMAKE_CXX_COMPILER "${GCC_BIN}/${GCC_PREFIX}g++.exe")
set(CMAKE_ASM_COMPILER "${GCC_BIN}/${GCC_PREFIX}gcc.exe")

set(ARCH_FLAGS "-march=rv32ec -mabi=ilp32e -mcmodel=medlow")
set(CMN_FLAGS   "${ARCH_FLAGS} -ffunction-sections -fdata-sections -fno-common")
set(CMAKE_C_FLAGS   "${CMN_FLAGS} -std=gnu99 -Os -g -Wall -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized --specs=nano.specs --specs=nosys.specs -Wl,-Bstatic" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "${ARCH_FLAGS} -x assembler-with-cpp -g -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized --specs=nano.specs --specs=nosys.specs -Wl,-Bstatic" CACHE STRING "" FORCE)

set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <LINK_FLAGS> <CMAKE_C_LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
