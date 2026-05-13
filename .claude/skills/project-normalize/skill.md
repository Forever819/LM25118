---
name: project-normalize
description: 解析 EIDE YAML 工程配置，诊断并规范化嵌入式项目的目录结构和文件组织
---

# Project Normalize Skill

解析 IDE 配置文件，扫描实际目录结构，诊断问题并自动修复，使嵌入式工程的文件组织符合规范标准。

## 规范标准 (Target Structure)

```
Src/
├── Core/                     # RISC-V 内核 (不变)
├── Debug/                    # 调试支持 (不变)
├── Peripheral/{inc,src}/     # MCU 外设库 (厂商标准库)
├── Startup/                  # 启动汇编
├── Ld/                       # 链接脚本
└── User/
    ├── main.c/h              # 入口
    ├── system_ch32v20x.c/h   # 系统初始化
    ├── ch32v20x_it.c/h       # 中断服务
    ├── ch32v20x_conf.h       # 外设配置头文件
    ├── app/                  # 应用层
    │   ├── inc/
    │   └── src/
    ├── bsp/                  # 板级驱动
    │   ├── inc/
    │   └── src/
    ├── uart/                 # UART 通信
    ├── usblib/               # USB 协议栈
    │   ├── config/{inc,src}/
    │   └── driver/{inc,src}/
    └── <其他单文件: .c + .h 成对放在同目录>
```

**命名规则:**
- 目录全小写，用 `-` 或 `_` 分隔 (不用大写)
- `inc/` 和 `src/` 统一小写
- 源文件名用 `snake_case`（如 `cmd_handler.c`, `ring_buffer.c`）
- 头文件用对应源文件的 `.h`，或模块名（如 `buckboost.h`）

---

## 执行流程

### 第一步：读取 EIDE 配置

查找并解析 `.eide/eide.yml`：

```bash
python3 -c "
import yaml, json
with open('.eide/eide.yml', 'r', encoding='utf-8') as f:
    cfg = yaml.safe_load(f)
print(json.dumps({
    'name': cfg.get('name'),
    'srcDirs': cfg.get('srcDirs', []),
    'targets': list(cfg.get('targets', {}).keys()),
    'incList': cfg.get('targets', {}).get('Debug', {}).get('cppPreprocessAttrs', {}).get('incList', []),
    'excludeList': cfg.get('targets', {}).get('Debug', {}).get('excludeList', []),
    'defineList': cfg.get('targets', {}).get('Debug', {}).get('cppPreprocessAttrs', {}).get('defineList', []),
}, indent=2))
"
```

如果没有 `.eide/eide.yml`，直接跳过到第二步（纯文件系统诊断）。

提取关键字段：
- `srcDirs`: 源文件根目录（如 `../../../Src`）
- `incList`: 头文件搜索路径列表
- `excludeList`: 排除的文件
- `defineList`: 预处理器宏定义

### 第二步：扫描实际目录结构

列出 `Src/` 下所有 `.c` 和 `.h` 文件，以及所有子目录：

```bash
find Src -type d | sort
```

用 Glob 或 Bash 统计：
- 每个目录下的 `.c` / `.h` 文件数量
- 空文件 (大小为 0)
- 含中文的路径

### 第三步：运行诊断规则

按 P0 → P1 → P2 优先级依次检查，每个问题输出一行。

#### P0-1: 目录大小写不一致

检查所有 `inc/` 和 `src/` 子目录是否统一小写。

```bash
find Src -type d -name '[A-Z]*' | grep -E '(inc|src|Inc|Src)'
```

**判定:** 存在 `Inc/` 或 `Src/`（首字母大写）即为违规。
**修复:** `git mv Src/User/BSP/Inc Src/User/bsp/inc_tmp && git mv Src/User/bsp/inc_tmp Src/User/bsp/inc`

#### P0-2: .gitignore 覆盖检查

检查 `.gitignore` 是否包含必要模式：

**必须覆盖的模式:**
```
build/
*.o
*.d
__pycache__/
```

检查方法：Read `.gitignore`，对比缺失项。

#### P1-1: 头/源文件归属检查

对每个 `.c` 文件，检查其 `#include` 的头文件是否在同一模块目录下。如果 `.h` 在其他模块目录，标记为分离。

**具体方法:** Grep 每个 `.c` 文件中的 `#include "xxx.h"`，检查该 `.h` 文件的实际位置与本模块 `inc/` 的关系。

**当前已知问题:**
- `BSP/src/hw_port.c` include `hw_port.h`，但 `hw_port.h` 在 `APP/inc/`

#### P1-2: 功能重复模块检查

检查是否有两个不同文件名实现相同/相似功能：

- `W25_Flash.h` (APP) vs `W25X20.h` (BSP) — 都是 W25 Flash 操作
- 两个文件命名风格不一致

**判定方法:** 对比文件名关键词（如 `W25`, `Flash`），以及头文件中的函数声明。

#### P1-3: 空文件检查

```bash
find Src -name "*.c" -o -name "*.h" | while read f; do
  sz=$(wc -c < "$f")
  if [ "$sz" -eq 0 ]; then echo "EMPTY: $f"; fi
done
```

#### P2-1: 中文路径检查

```bash
find . -name '*[一-龥]*' 2>/dev/null
```

#### P2-2: 命名风格混用检查

扫描 `APP/inc/` 和 `APP/src/` 下的文件名：

| 风格 | 示例 |
|------|------|
| snake_case | `buckboost.c`, `cmd_handler.c` |
| PascalCase | `RingBufferQueue.c`, `ParamSyncMgr.c` |
| 混合 | `DMA_Mutex.c`, `LCD_GUI.c` |

**报告混用情况，建议统一为 snake_case。**

#### P2-3: 目录命名规范检查

- `USBLIB/CONFIG/` → 全大写，应改为 `config/`
- `USBLIB/USB-Driver/` → 连字符+大写，应改为 `driver/`
- `APP/` → 全大写，应改为 `app/`
- `BSP/` → 全大写，应改为 `bsp/`
- `UART/` → 全大写，应改为 `uart/`

### 第四步：生成诊断报告

格式：

```
╔══════════════════════════════════════════════╗
║  工程结构诊断报告: <工程名>                    ║
╠══════════════════════════════════════════════╣
║  配置文件:  IDE/EIDE/CH32V203/.eide/eide.yml ║
║  源文件数:  N                                ║
║  头文件数:  M                                ║
╠══════════════════════════════════════════════╣
║  P0 阻塞级:  X 个问题                        ║
║  P1 结构级:  Y 个问题                        ║
║  P2 规范级:  Z 个问题                        ║
╠══════════════════════════════════════════════╣
║  (逐项列出每个问题的详情和建议)               ║
╚══════════════════════════════════════════════╝
```

对每个问题输出：**[级别] 文件路径 — 问题描述 → 建议修复**

### 第五步：执行修复

**原则:** 每个修复操作前展示将要执行的命令，等用户确认。P0 优先修复，P1/P2 逐项确认。

#### 修复操作清单

**重命名目录 (git mv):**
```
Src/User/APP      → Src/User/app
Src/User/BSP      → Src/User/bsp
Src/User/BSP/Inc  → Src/User/bsp/inc
Src/User/BSP/Src  → Src/User/bsp/src
Src/User/UART     → Src/User/uart
Src/User/USBLIB/CONFIG    → Src/User/usblib/config
Src/User/USBLIB/USB-Driver → Src/User/usblib/driver
```

Windows 下 git mv 需要两步（大小写不敏感文件系统）：
```bash
git mv Src/User/BSP/Inc Src/User/BSP/inc_
git mv Src/User/BSP/inc_ Src/User/BSP/inc
```

**移动错位文件:**
```
Src/User/APP/inc/hw_port.h → Src/User/BSP/inc/hw_port.h  (跟 hw_port.c 放一起)
```

**删除空/死文件:**
```
Src/User/BSP/Inc/LCD_pic.h  (空文件)
```

**更新 .gitignore:**
添加缺失的模式行。

#### 第六步：更新 EIDE 配置

修复目录后，必须同步更新 `.eide/eide.yml` 中的 `incList` 路径：

| 旧路径 | 新路径 |
|--------|--------|
| `Src/User/BSP/inc` | `Src/User/bsp/inc` |
| `Src/User/APP/inc` | `Src/User/app/inc` |
| `Src/User/USBLIB/USB-Driver/inc` | `Src/User/usblib/driver/inc` |
| `Src/User/USBLIB/Config/inc` | `Src/User/usblib/config/inc` |
| `Src/User/UART` | `Src/User/uart` |

同样更新 `excludeList` 中的路径。

同时检查并更新：
- `IDE/CMake/CMakeLists.txt` 中的 `target_include_directories` 路径
- `IDE/MRS/CH32V203_Application/.cproject` 中的 include 路径（XML 格式）

### 第七步：验证

修复后重新运行诊断，确认所有 P0 问题清零。

```bash
git status  # 确认变更范围
git diff --stat  # 查看统计
```

---

## 使用方式

用户说：
- "规范化项目结构"
- "检查工程目录规范"
- "project normalize"
- "/project-normalize"

---

## 复用规则

诊断和修复逻辑独立于具体工程：
1. 目录命名规范 (大小写、连字符) → 通用
2. 头/源文件归属 → 通用（检查 `#include` 和实际位置）
3. 空文件检测 → 通用
4. .gitignore 覆盖 → 通用（模式可配置）
5. 中文路径 → 通用
6. EIDE YAML 解析 → EIDE 专用
7. CMakeLists.txt 更新 → CMake 专用
8. Eclipse .cproject 更新 → MRS 专用

新增 IDE 格式支持时，只需增加对应的「配置解析」和「配置更新」两个模块。
