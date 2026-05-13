---
name: eide-flash-debug
description: 在 VS Code EIDE 环境中配置 CH32 RISC-V 系列 MCU 的 wch-openocd 烧录和 Cortex-Debug GDB 调试
---

# EIDE Flash & Debug Skill

CH32 RISC-V 系列 MCU (CH32V00x/CH32V30x 等) 在 VS Code + EIDE 环境中的烧录与 GDB 调试配置指南。

## 环境要求

- **VS Code 扩展**: EIDE, Cortex-Debug
- **调试器**: WCH-LinkE (SWD/SDI 接口)
- **驱动**: WCH 官方驱动 (MRS2 安装自带, 或 `WCHLinkDrv_WHQL_S.exe` 安装)
- **工具链**: MounRiver Studio 2 (提供 `riscv-wch-elf-gdb.exe` 和 SVD 外设描述文件)

---

## 一、烧录配置 (wch-openocd)

### 1.1 所需文件

| 文件 | 路径 | 用途 |
|------|------|------|
| OpenOCD | `IDE/EIDE/tools/wch_openocd/bin/openocd.exe` | WCH 版 OpenOCD 可执行文件 |
| 目标配置 | `IDE/EIDE/wch-riscv.cfg` | WCH-LinkE + CH32 RISC-V 芯片配置 |
| 烧录脚本 | `IDE/EIDE/download.bat` | 全片擦除 + 编程 + 校验 + 复位 |

### 1.2 wch-riscv.cfg 关键配置

```tcl
adapter driver wlinke          # WCH-LinkE 调试器驱动
adapter speed 6000             # SDI 通信速率 6MHz
transport select sdi           # 单线调试接口
wlink_set_address 0x00000000   # Flash 基地址
# 目标芯片
set _CHIPNAME wch_riscv
sdi newtap $_CHIPNAME cpu -irlen 5 -expected-id 0x00001
target create $_TARGETNAME.0 wch_riscv -chain-position $_TARGETNAME
$_TARGETNAME.0 configure -work-area-phys 0x20000000 -work-area-size 10000 -work-area-backup 1
# Flash bank
flash bank $_FLASHNAME wch_riscv 0x00000000 0 0 0 $_TARGETNAME.0
```

### 1.3 download.bat 烧录流程

```batch
openocd -f ./wch-riscv.cfg -c init -c halt ^
  -c "flash erase_sector wch_riscv 0 last" ^
  -c "program %HEXFILE%" ^
  -c "verify_image %HEXFILE%" ^
  -c wlink_reset_resume -c exit
```

步骤: 初始化 OpenOCD → 停止内核 → 全片擦除 → 编程 HEX → 校验 → 复位运行

### 1.4 EIDE 项目配置 (eide.yml)

```yaml
uploadConfigMap:
  Custom:
    commandLine: .\download.bat "${hexFile}"   # EIDE 构建后自动调用
uploader: Custom                               # 激活 Custom 烧录器
```

**在 EIDE 中**: 点击工具栏闪电图标 (Upload) 即可一键烧录。EIDE 先编译生成 `.hex` 文件，再调用 `download.bat`。

**命令行烧录** (替代方式):
```bash
# 在 IDE/EIDE/ 目录下执行
openocd -f ./wch-riscv.cfg -c init -c halt \
  -c "flash erase_sector wch_riscv 0 last" \
  -c "program build/obj/CH32V006.hex" \
  -c "verify_image build/obj/CH32V006.hex" \
  -c wlink_reset_resume -c exit
```

---

## 二、GDB 调试配置 (Cortex-Debug)

### 2.1 launch.json 配置

文件位于 `Src/.vscode/launch.json` (Cortex-Debug 扩展使用):

```json
{
    "cwd": "${workspaceRoot}",
    "executable": "../IDE/EIDE/build/obj/CH32V006.elf",
    "name": "Debug with OpenOCD",
    "request": "launch",
    "type": "cortex-debug",
    "servertype": "openocd",
    "runToEntryPoint": "main",
    "showDevDebugOutput": "none",
    "device": "CH32V00Xxx",
    "svdFile": "<MRS2安装目录>/.../CH32V00Xxx.svd",
    "gdbPath": "<WCH-GCC12>/bin/riscv-wch-elf-gdb.exe",
    "configFiles": [
        "../IDE/EIDE/wch-riscv.cfg"
    ]
}
```

### 2.2 关键字段说明

| 字段 | 值 | 说明 |
|------|------|------|
| `type` | `cortex-debug` | VS Code 调试扩展类型 |
| `servertype` | `openocd` | 调试服务器使用 OpenOCD |
| `executable` | `../IDE/EIDE/build/obj/CH32V006.elf` | 指向 EIDE 构建产物 (ELF) |
| `gdbPath` | `riscv-wch-elf-gdb.exe` | WCH 专用 GDB (MRS2 自带) |
| `configFiles` | `../IDE/EIDE/wch-riscv.cfg` | OpenOCD 目标配置 (复用烧录配置) |
| `svdFile` | `CH32V00Xxx.svd` | 外设寄存器描述文件 (调试时查看寄存器) |
| `runToEntryPoint` | `main` | 自动在 main() 断点 |
| `device` | `CH32V00Xxx` | 设备标识 |

### 2.3 调试启动流程

1. **先构建**: EIDE 中点击 Build (锤子图标) 或 VS Code 中运行 build task
2. **启动调试**: VS Code `F5` 或 "Run and Debug" 面板选择 "Debug with OpenOCD"
3. **OpenOCD 自动启动**: Cortex-Debug 自动启动 `openocd.exe -f ../IDE/EIDE/wch-riscv.cfg`
4. **GDB 连接**: `riscv-wch-elf-gdb.exe` 通过 `localhost:3333` 连接 OpenOCD
5. **自动断点**: 程序在 `main()` 处停止, 可以单步、查看变量、寄存器等

---

## 三、备用方案: wlink

### 3.1 环境

- **驱动**: WinUSB (用 Zadig 安装, 替换 WCH 官方驱动)
- **工具**: `Doc/About_EIDE_Flash/wlink/wlink.exe`

### 3.2 EIDE 配置

```yaml
uploadConfigMap:
  Custom:
    commandLine: .\wlink.exe flash --address 0x08000000 "${hexFile}"
```

### 3.3 驱动切换

**wch-openocd → wlink**: Zadig 管理员运行 → List All Devices → 选 WCH-Link (Interface 0) → WinUSB → Replace Driver

**wlink → wch-openocd**: 设备管理器卸载 WCH-Link → 勾选"删除驱动程序软件" → 重新插拔自动安装官方驱动

> 两种驱动不能共存。推荐日常开发使用 wch-openocd (与 MRS2 兼容, 支持调试)。

---

## 四、文件索引

| 类别 | 路径 | 说明 |
|------|------|------|
| EIDE 项目配置 | `IDE/EIDE/.eide/eide.yml` | 工具链、构建、烧录配置 |
| 构建 Makefile | `IDE/EIDE/Makefile` | EIDE 生成的独立 Makefile |
| 烧录脚本 | `IDE/EIDE/download.bat` | OpenOCD 烧录命令 |
| OpenOCD 配置 | `IDE/EIDE/wch-riscv.cfg` | WCH-LinkE 目标芯片配置 |
| OpenOCD 工具 | `IDE/EIDE/tools/wch_openocd/` | WCH 版 OpenOCD 完整发行版 |
| VS Code 调试 | `Src/.vscode/launch.json` | Cortex-Debug GDB 调试配置 |
| VS Code 任务 | `IDE/EIDE/.vscode/tasks.json` | EIDE 构建/烧录任务 |
| 烧录文档 | `Doc/About_EIDE_Flash/Readme.md` | 烧录方案对比与驱动切换说明 |
| CMake 构建 | `IDE/CMAKE/` | 自动生成的 CMake 构建系统 |
