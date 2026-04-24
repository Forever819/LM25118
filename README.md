# LM25118 数控电源

基于 **TI LM25118** 同步降压-升压控制器和 **WCH CH32V006** RISC-V 微控制器的数控可调电源。支持恒压(CV)/恒流(CC)模式切换，具备完善的保护功能和直观的 OLED 人机交互界面。

## 硬件平台

| 组件 | 型号 | 说明 |
|------|------|------|
| MCU | CH32V006 (RISC-V RV32EC, 48MHz) | 62KB Flash / 8KB RAM |
| 电源控制 | LM25118 | 宽输入同步降压-升压控制器 |
| 显示 | SSD1306 0.96" OLED (128x64) | I2C 接口 |
| 输入 | 旋转编码器 + 按键 | 参数调节与操作 |
| 指示灯 | WS2812 RGB LED | 状态指示 |
| 温度传感 | NTC 热敏电阻 | 电感/MOS管温度监测 |

## 功能特性

### 控制模式
- **恒压模式 (CV)** — 通过增量式 PID 闭环调节输出电压
- **恒流模式 (CC)** — 电流限制模式，PID 电流环前馈补偿
- **空闲模式 (IDLE)** — 输出关断，PWM 停止

### 保护功能
- **OVP** — 过压保护
- **OCP** — 过流保护
- **SCP** — 短路保护
- **OVT** — 过温保护 (阈值 80°C)

所有保护触发后进入故障锁定状态，需手动确认后恢复。

### 用户界面
- OLED 实时显示：输入/输出电压、电流、功率、效率、温度
- 旋转编码器调节：电压/电流/功率设定值，支持逐位微调
- WS2812 LED 状态指示：空闲(绿)、CV(红)、CC(金)、故障(白)
- 蜂鸣器提示音与故障报警
- 长按按键保存参数至 Flash

## 系统架构

```
                    +------------------+
  ADC 通道 (DMA):   |  Vin (Ch0)      |
  5通道循环采样     |  Iin (Ch1)      |
                    |  Vout (Ch2)     |
                    |  Iout (Ch3)     |
                    |  NTC (Ch4)      |
                    +--------+---------+
                             |
                    IIR 滤波 + 均值滤波
                             |
                    +--------v---------+
                    |   PID 控制器     |
                    |   - 电压环       |-----> TIM1 OC3 PWM (占空比)
                    |   - 电流环       |            |
                    +--------+---------+      LM25118
                             |             Buck-Boost
                    +--------v---------+
                    |    保护逻辑      |
                    | OVP / OCP / SCP / OVT |
                    +------------------+
```

- **PWM 频率**: 5 kHz (TIM1, RepetitionCounter 分频至 ~1kHz 控制环路)
- **滤波**: IIR 低通滤波 + 滑动均值滤波
- **通信**: I2C (OLED, 800kHz) / SPI (WS2812) / UART (调试输出)

## 项目结构

```
LM25118_PowerModule/
├── Src/
│   ├── main.c                  # 主程序，状态机，保护逻辑
│   ├── ch32v00X_it.c           # 中断服务程序
│   ├── Startup/                # RISC-V 启动文件
│   ├── Ld/                     # 链接脚本
│   ├── Debug/                  # UART 调试输出
│   ├── Peripheral/             # WCH 外设驱动库
│   └── BSP/
│       ├── Inc/                # 头文件
│       └── Src/
│           ├── ADC_k.c         # ADC + DMA 多通道采样，IIR/均值滤波
│           ├── PID.c           # 增量式 PID 控制器
│           ├── Timer_k.c       # TIM1 PWM, TIM2 蜂鸣器
│           ├── Encoder_k.c     # 旋转编码器 + 按键驱动
│           ├── OLED.c          # SSD1306 驱动 (I2C)
│           ├── OELD_UI.c       # 电源 UI 界面
│           ├── WS2812_k.c      # WS2812 RGB LED (SPI)
│           ├── Buzzer_k.c      # 蜂鸣器队列 (PWM)
│           └── flash_param.c   # Flash 参数存储
└── IDE/
    ├── EIDE/                   # VS Code EIDE 工程
    └── MRS/                    # MounRiver Studio 工程
```

## 开发环境

支持两种 IDE：

- **EIDE (VS Code)** — 使用 `IDE/EIDE/` 下的工程配置
- **MounRiver Studio** — 使用 `IDE/MRS/` 下的工程配置

工具链：RISC-V GCC，目标架构 `rv32ec`，ABI `ilp32e`，编译优化 `-Os`。

### 编译与烧录

1. 使用 VS Code + EIDE 插件或 MounRiver Studio 打开对应工程
2. 编译生成 `.hex` / `.bin` 固件
3. 通过 WCH-Link 烧录（烧录配置见 `IDE/EIDE/烧录配置/`）

## 通信与调试

- **UART**: 115200 baud，输出 Vin/Vout 等实时数据
- **OLED**: I2C 接口，显示完整的电源运行参数
- **WS2812**: 通过 SPI 模拟时序驱动单颗 RGB LED

## License

MIT
