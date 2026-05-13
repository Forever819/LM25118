---
name: eide-build
description: 解析 EIDE 工程 YAML 配置, 生成 CMake 文件并编译, 仅返回退出码
---

# EIDE Build Skill

解析 EIDE 工程配置 (`.eide/eide.yml`) → 生成 CMake 文件 → 编译 → 返回退出码。

**定位**: 特定化的编译测试接口。只处理 EIDE 类型工程, 不做多 IDE 兼容。

## 调用方式

```
python .claude/skills/eide-build/build.py <project_root> [--verbose] [--dry-run] [--toolchain-bin <dir>] [--toolchain-prefix <prefix>]
```

- `project_root` — 工程根目录 (默认 `.`)
- `--verbose` / `-v` — 输出完整编译日志
- `--dry-run` — 仅生成 CMake 文件, 不编译
- `--toolchain-bin` — 工具链 bin 目录 (覆盖自动检测)
- `--toolchain-prefix` — 工具链前缀 (默认 `riscv-none-embed-`)

## 退出码语义

| 码   | 含义                                              | AI 行为          |
| --- | ----------------------------------------------- | -------------- |
| 0   | 编译成功                                            | 报告成功           |
| 1   | 配置错误 (找不到 eide.yml / YAML 解析失败 / 源文件缺失 / 工具链缺失) | 报告具体错误信息, 建议修复 |
| 2   | CMake 配置失败                                      | 报告 cmake 错误    |
| 3   | 编译失败 (有编译错误/警告)                                 | 报告编译错误数量       |

## 工作流程

```
[1. 扫描]  在 IDE/EIDE/*/.eide/ 下搜索 eide.yml → 解析 YAML 提取构建参数
[2. 生成]  扫描源文件目录 → 生成 IDE/CMake/CMakeLists.txt + toolchain_eide.cmake
[3. 编译]  cmake -S ... -B ... -G Ninja → cmake --build ... → 返回退出码
```

## AI 调用方注意事项

1. 如果退出码非 0, stderr 中已包含错误详情, 直接展示给用户即可
2. 工具链自动检测顺序: `--toolchain-bin` 参数 → PATH 环境变量 → MounRiver Studio 默认安装路径
3. 生成的文件位于 `IDE/CMake/` 下, 构建产物在 `IDE/CMake/build/`
4. 若需调试 CMake 生成结果, 先用 `--dry-run` 查看生成的 CMakeLists.txt
