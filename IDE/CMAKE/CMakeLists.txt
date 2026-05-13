# EIDE Build Skill — 自动生成的 CMakeLists.txt
cmake_minimum_required(VERSION 3.21)
project(CH32V006 C ASM)

set(PROJECT_ROOT "C:/Users/Kun/Desktop/Project/LM25118")

include_directories(
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Core"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Debug"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/inc"
    "C:/Users/Kun/Desktop/Project/LM25118/Src"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Inc"
)

add_executable(${CMAKE_PROJECT_NAME}
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Startup/startup_ch32v00x.S"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/ADC_k.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/Buzzer_k.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/Encoder_k.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/OELD_UI.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/OLED.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/OLED_Data.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/PID.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/Timer_k.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/WS2812_k.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/bm_img.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/BSP/Src/flash_param.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Core/core_riscv.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Debug/debug.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_adc.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_dbgmcu.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_dma.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_exti.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_flash.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_gpio.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_i2c.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_iwdg.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_misc.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_opa.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_pwr.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_rcc.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_spi.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_tim.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_usart.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/Peripheral/src/ch32v00x_wwdg.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/ch32v00X_it.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/main.c"
    "C:/Users/Kun/Desktop/Project/LM25118/Src/system_ch32v00X.c"
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -march=rv32ec -mabi=ilp32e -mcmodel=medlow
    -T "C:/Users/Kun/Desktop/Project/LM25118/Src/Ld/Link.ld"
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    --specs=nano.specs
    --specs=nosys.specs
    -Xlinker
    --gc-sections
    -nostartfiles
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE printfloat)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE m)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND "C:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin/riscv-none-embed-objcopy.exe" -O ihex
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
            "${CMAKE_PROJECT_NAME}.hex"
    COMMAND "C:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin/riscv-none-embed-objcopy.exe" -O binary
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
            "${CMAKE_PROJECT_NAME}.bin"
    COMMAND "C:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin/riscv-none-embed-size.exe"
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
    COMMENT "Generating .hex, .bin, and size report"
)
