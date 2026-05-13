# CH32 EIDE 烧录方案总结

## 方案一：wlink（开源工具）

### 环境要求

- 驱动：**WinUSB**（用 Zadig 安装）
- 工具：`wlink.exe`

### EIDE 烧录命令

```
.\wlink.exe flash --address 0x08000000 "${hexFile}"
```

### 优点

- 配置简单，无需 cfg 文件
- 与 MRS2 官方 IDE 无关

### 缺点

- 与 MRS2 / wch-openocd **驱动冲突**，无法共存

---

## 方案二：wch-openocd

### 环境要求

- 驱动：**WCH 官方驱动**（MRS2 安装后自带，或插拔设备让 Windows 自动安装），一般为wch-linkutility程序文件夹里的WCHLinkDrv_WHQL_S.exe
- 工具：`openocd.exe` + `wch-riscv.cfg`

### download.bat

```batch
@echo off
set "HEXFILE=%~1"
set "HEXFILE=%HEXFILE:\=/%"
openocd -f ./wch-riscv.cfg -c init -c halt -c "flash erase_sector wch_riscv 0 last" -c "program %HEXFILE%" -c "verify_image %HEXFILE%" -c wlink_reset_resume -c exit
```

### EIDE 烧录命令

```
.\download.bat "${hexFile}"
```

### 优点

- 与 MRS2 共用同一套驱动，兼容性好
- openocd 功能更完整（支持调试等）

### 缺点

- 需要额外的 cfg 文件
- wlink 开源工具无法同时使用

---

## 驱动对照表

| 工具                 | 所需驱动     | 安装方式                              |
| ------------------ | -------- | --------------------------------- |
| wlink              | WinUSB   | Zadig → 选 WinUSB → Replace Driver |
| wch-openocd / MRS2 | WCH 官方驱动 | 设备管理器卸载后重新插拔自动安装                  |

> ⚠️ 两种驱动**不能共存**，切换方案必须切换驱动

---

## 切换驱动方法

### 官方驱动 → WinUSB（切换到 wlink）

1. 以管理员身份运行 Zadig
2. `Options` → `List All Devices`
3. 选择 `WCH-Link (Interface 0)`
4. 右侧选 `WinUSB` → `Replace Driver`

### WinUSB → 官方驱动（切换到 openocd）

1. 设备管理器找到 WCH-Link
2. 右键 → 卸载设备 → 勾选"删除驱动程序软件"
3. 拔插 WCH-Link，Windows 自动重装官方驱动

---

## 推荐

| 场景              | 推荐方案                 |
| --------------- | -------------------- |
| 只用 EIDE，不用 MRS2 | 方案一 wlink，简单省事       |
| 需要同时用 MRS2 调试   | 方案二 wch-openocd，驱动统一 |
