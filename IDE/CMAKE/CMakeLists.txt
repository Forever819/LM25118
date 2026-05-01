# EIDE Build Skill — CMakeLists.txt
# NOTE: toolchain paths (C:/MounRiver/...) are machine-specific.
# Adjust GCC_BIN and objcopy/size paths to your local MRS installation.
cmake_minimum_required(VERSION 3.21)
project(CH32V006 C ASM)

# 项目根目录（相对此文件位置：IDE/CMAKE/ → ../..）
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
get_filename_component(PROJECT_ROOT "${PROJECT_ROOT}" ABSOLUTE)

include_directories(
    "${PROJECT_ROOT}/Src/Core"
    "${PROJECT_ROOT}/Src/Debug"
    "${PROJECT_ROOT}/Src/Peripheral/inc"
    "${PROJECT_ROOT}/Src"
    "${PROJECT_ROOT}/Src/BSP/inc"
    "${PROJECT_ROOT}/Src/App"
)

add_executable(${CMAKE_PROJECT_NAME}
    "${PROJECT_ROOT}/Src/Startup/startup_ch32v00x.S"
    "${PROJECT_ROOT}/Src/App/Buzzer_Responder.c"
    "${PROJECT_ROOT}/Src/App/Digital_Power_Core.c"
    "${PROJECT_ROOT}/Src/App/EffTest.c"
    "${PROJECT_ROOT}/Src/App/Event_Bus.c"
    "${PROJECT_ROOT}/Src/App/LED_Responder.c"
    "${PROJECT_ROOT}/Src/App/OLED_UI.c"
    "${PROJECT_ROOT}/Src/App/PID.c"
    "${PROJECT_ROOT}/Src/App/UI_Responder.c"
    "${PROJECT_ROOT}/Src/BSP/src/ADC.c"
    "${PROJECT_ROOT}/Src/BSP/src/Buzzer.c"
    "${PROJECT_ROOT}/Src/BSP/src/Encoder.c"
    "${PROJECT_ROOT}/Src/BSP/src/Filter.c"
    "${PROJECT_ROOT}/Src/BSP/src/FloatEditor.c"
    "${PROJECT_ROOT}/Src/BSP/src/OLED.c"
    "${PROJECT_ROOT}/Src/BSP/src/OLED_Data.c"
    "${PROJECT_ROOT}/Src/BSP/src/Timer.c"
    "${PROJECT_ROOT}/Src/BSP/src/WS2812.c"
    "${PROJECT_ROOT}/Src/BSP/src/bm_img.c"
    "${PROJECT_ROOT}/Src/BSP/src/flash_param.c"
    "${PROJECT_ROOT}/Src/Core/core_riscv.c"
    "${PROJECT_ROOT}/Src/Debug/debug.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_adc.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_dbgmcu.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_dma.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_exti.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_flash.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_gpio.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_i2c.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_iwdg.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_misc.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_opa.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_pwr.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_rcc.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_spi.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_tim.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_usart.c"
    "${PROJECT_ROOT}/Src/Peripheral/src/ch32v00x_wwdg.c"
    "${PROJECT_ROOT}/Src/ch32v00X_it.c"
    "${PROJECT_ROOT}/Src/main.c"
    "${PROJECT_ROOT}/Src/system_ch32v00X.c"
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -march=rv32ec -mabi=ilp32e -mcmodel=medlow
    -T "${PROJECT_ROOT}/Src/Ld/Link.ld"
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    --specs=nano.specs
    --specs=nosys.specs
    -Xlinker
    --gc-sections
    -nostartfiles
    -Xlinker
    -u
    -Xlinker
    _printf_float
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE printfloat)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE m)

# Toolchain post-build commands — adjust paths to your MRS install
set(TOOLCHAIN_BIN "C:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND "${TOOLCHAIN_BIN}/riscv-none-embed-objcopy.exe" -O ihex
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
            "${CMAKE_PROJECT_NAME}.hex"
    COMMAND "${TOOLCHAIN_BIN}/riscv-none-embed-objcopy.exe" -O binary
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
            "${CMAKE_PROJECT_NAME}.bin"
    COMMAND "${TOOLCHAIN_BIN}/riscv-none-embed-size.exe"
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
    COMMENT "Generating .hex, .bin, and size report"
)
