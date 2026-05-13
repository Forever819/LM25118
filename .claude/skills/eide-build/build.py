#!/usr/bin/env python3
"""
EIDE Build Skill — 解析 EIDE 工程 YAML 配置，生成 CMake 文件并编译。
只向 AI 调用方返回退出码，不输出结构化数据。

退出码:
  0 — 编译成功
  1 — 配置错误（找不到 eide.yml / YAML 解析失败 / 源文件缺失）
  2 — CMake 配置失败
  3 — 编译失败

用法:
  python build.py <project_root> [--verbose] [--toolchain-bin <dir>] [--toolchain-prefix <prefix>]
"""

import re
import sys
import os
import shutil
import subprocess
from pathlib import Path
from typing import Optional, List, Tuple, Dict, Any

# Windows 下强制 UTF-8 输出，避免中文乱码
if sys.platform == "win32":
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

try:
    import yaml
except ImportError:
    sys.stderr.write("FATAL: PyYAML is required. Install with: pip install pyyaml\n")
    sys.exit(1)


# ---------------------------------------------------------------------------
# 工具链默认搜索路径（Windows）
# ---------------------------------------------------------------------------
_TOOLCHAIN_SEARCH_PATHS = [
    "C:/MounRiver/MounRiver_Studio2/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin",
    "C:/MounRiver/MounRiver_Studio/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin",
]


# ===================================================================
# 扫描阶段
# ===================================================================

def find_eide_config(project_root: Path) -> Path:
    """在 IDE/EIDE 子目录下搜索 eide.yml。

    Returns:
        匹配到的 eide.yml 绝对路径。

    Raises:
        FileNotFoundError: 未找到任何 eide.yml。
    """
    eide_dir = project_root / "IDE" / "EIDE"
    if not eide_dir.is_dir():
        raise FileNotFoundError(f"EIDE 目录不存在: {eide_dir}")

    candidates = list(eide_dir.rglob("eide.yml"))
    if not candidates:
        raise FileNotFoundError(f"在 {eide_dir} 下未找到任何 eide.yml")

    # 如果有多个，优先选择路径最短的（通常在 IDE/EIDE/<name>/.eide/ 下）
    candidates.sort(key=lambda p: len(p.parts))
    return candidates[0]


def parse_eide_config(yaml_path: Path) -> Dict[str, Any]:
    """解析 EIDE YAML 配置，提取构建所需字段。

    Args:
        yaml_path: eide.yml 的绝对路径。

    Returns:
        包含所有构建参数的字典。关键键:
          - name:           项目名
          - src_dirs:       源文件目录（绝对路径列表）
          - inc_dirs:       头文件目录（绝对路径列表）
          - defines:        预处理器宏定义列表
          - exclude_files:  排除的文件（绝对路径列表）
          - arch:           RISC-V 架构字符串
          - abi:            ABI 字符串
          - code_model:     代码模型
          - c_std:          C 语言标准
          - optimization:   优化级别 GCC 标志
          - misc_flags:     附加编译标志
          - asm_flags:      汇编器标志
          - ld_flags:       链接器标志（含 -T 脚本路径，路径已解析为绝对路径）
          - lib_flags:      库标志
          - linker_script:  链接脚本绝对路径
    """
    with open(yaml_path, "r", encoding="utf-8") as fh:
        raw = yaml.safe_load(fh)

    if raw is None:
        raise ValueError(f"YAML 文件为空: {yaml_path}")

    # EIDE 路径解析基准是 .eide/ 的父目录（即 EIDE 工程根目录）
    base_dir = yaml_path.parent.parent

    # --- 项目名 ---
    name = raw.get("name", "EIDE_Project")

    # --- 源文件目录 ---
    src_dirs_rel = raw.get("srcDirs", [])
    src_dirs = [_resolve_rel(base_dir, d) for d in src_dirs_rel]

    # --- 取第一个 target ---
    targets = raw.get("targets", {})
    if not targets:
        raise ValueError("eide.yml 中未定义任何 target")
    target_name = next(iter(targets))
    target = targets[target_name]

    # --- include / defines ---
    cpp_attrs = target.get("cppPreprocessAttrs", {})
    inc_dirs_rel = cpp_attrs.get("incList", [])
    inc_dirs = [_resolve_rel(base_dir, d) for d in inc_dirs_rel]
    defines = cpp_attrs.get("defineList", [])

    # --- 排除文件 ---
    exclude_files_rel = target.get("excludeList", [])
    exclude_files = [_resolve_rel(base_dir, f) for f in exclude_files_rel]

    # --- 工具链名 ---
    toolchain_name = target.get("toolchain", "")

    # --- 工具链选项 ---
    tcm = target.get("toolchainConfigMap", {})
    tc_options: Dict[str, Any] = {}
    if toolchain_name and toolchain_name in tcm:
        tc_opts = tcm[toolchain_name].get("options", {})
    elif tcm:
        # 取第一个可用工具链
        first_tc = next(iter(tcm.values()))
        tc_opts = first_tc.get("options", {})
    else:
        tc_opts = {}

    # --- 解析各子选项 ---
    global_opts = tc_opts.get("global", {})
    cc_opts = tc_opts.get("c/cpp-compiler", {})
    asm_opts = tc_opts.get("asm-compiler", {})
    ld_opts = tc_opts.get("linker", {})

    arch = global_opts.get("arch", "rv32imac")
    abi = global_opts.get("abi", "ilp32")
    code_model = global_opts.get("code-model", "medlow")
    misc_flags = global_opts.get("misc-control", "")
    debug_info = global_opts.get("output-debug-info", "enable")

    c_std = cc_opts.get("language-c", "gnu99")
    optimization_eide = cc_opts.get("optimization", "level-size")
    c_flags_extra = cc_opts.get("C_FLAGS", "")

    asm_flags_extra = asm_opts.get("ASM_FLAGS", "")

    ld_flags_raw = ld_opts.get("LD_FLAGS", "")
    lib_flags = ld_opts.get("LIB_FLAGS", "")
    gc_sections = ld_opts.get("remove-unused-input-sections", True)

    # --- 链接脚本路径：可能直接配置，也可能嵌在 LD_FLAGS 里 ---
    linker_script_rel = ld_opts.get("linkerScriptPath", "")
    linker_script = ""
    if linker_script_rel:
        linker_script = _resolve_rel(base_dir, linker_script_rel)

    # --- 映射优化级别 ---
    opt_map = {
        "level-size": "-Os",
        "level-speed": "-O2",
        "level-0": "-O0",
        "level-1": "-O1",
        "level-2": "-O2",
        "level-3": "-O3",
    }
    optimization = opt_map.get(optimization_eide, "-Os")

    # --- 构建 C 标准标志 ---
    c_std_flag = f"-std={c_std}" if c_std else ""

    # --- 调试标志 ---
    debug_flag = "-g" if debug_info == "enable" else ""

    # --- 链接脚本路径解析（从 LD_FLAGS 中提取 -T <path> 并移除）---
    ld_flags_resolved = _resolve_ld_script_in_flags(ld_flags_raw, base_dir)
    if not linker_script:
        linker_script = _extract_ld_script_from_flags(ld_flags_resolved)
    # 从 ld_flags 中剥离 -T 及其路径，避免 CMakeLists.txt 中重复
    ld_flags_resolved = _strip_ld_script_from_flags(ld_flags_resolved)

    # 从 misc_flags 中提取 --specs 标志，这些在链接时必须传递给 gcc 驱动
    specs_flags = _extract_specs_flags(misc_flags)
    if specs_flags:
        ld_flags_resolved = specs_flags + " " + ld_flags_resolved

    # --- 如果 remove-unused-input-sections 为 true 但 LD_FLAGS 未含 gc-sections，补充 ---
    if gc_sections and "-gc-sections" not in ld_flags_resolved:
        ld_flags_resolved += " -Wl,--gc-sections"

    # 清理首尾空白
    ld_flags_resolved = ld_flags_resolved.strip()

    return {
        "name": name,
        "src_dirs": src_dirs,
        "inc_dirs": inc_dirs,
        "defines": defines,
        "exclude_files": exclude_files,
        "arch": arch,
        "abi": abi,
        "code_model": code_model,
        "c_std_flag": c_std_flag,
        "optimization": optimization,
        "debug_flag": debug_flag,
        "misc_flags": misc_flags,
        "c_flags_extra": c_flags_extra,
        "asm_flags_extra": asm_flags_extra,
        "ld_flags": ld_flags_resolved,
        "lib_flags": lib_flags,
        "linker_script": str(linker_script) if linker_script else "",
    }


# ===================================================================
# 源文件扫描
# ===================================================================

def scan_sources(src_dirs: List[Path], exclude_files: List[Path]) -> Tuple[List[Path], List[Path]]:
    """递归扫描源目录，收集 .c / .S 文件。

    Args:
        src_dirs:      源文件目录列表。
        exclude_files: 排除文件列表。

    Returns:
        (c_sources, asm_sources) 两个 Path 列表。
    """
    exclude_set = {f.resolve() for f in exclude_files}

    c_sources: List[Path] = []
    asm_sources: List[Path] = []

    for src_dir in src_dirs:
        if not src_dir.is_dir():
            continue
        for root, _dirs, files in os.walk(src_dir):
            for fname in files:
                fpath = Path(root, fname).resolve()
                if fpath in exclude_set:
                    continue
                suffix = fpath.suffix.lower()
                if suffix == ".c":
                    c_sources.append(fpath)
                elif suffix == ".s":
                    asm_sources.append(fpath)

    # 按路径排序，保证可复现
    c_sources.sort(key=lambda p: str(p))
    asm_sources.sort(key=lambda p: str(p))
    return c_sources, asm_sources


# ===================================================================
# 工具链检测
# ===================================================================

def detect_toolchain(toolchain_bin: Optional[str] = None,
                     toolchain_prefix: str = "riscv-none-embed-") -> Tuple[str, str]:
    """检测 RISC-V GCC 工具链。

    Args:
        toolchain_bin:    用户指定的 bin 目录（可选）。
        toolchain_prefix: 工具链前缀。

    Returns:
        (gcc_bin_dir, prefix) — bin 目录和前缀。

    Raises:
        FileNotFoundError: 未找到可用工具链。
    """
    gcc_name = f"{toolchain_prefix}gcc.exe"

    # 1. 用户指定的路径
    if toolchain_bin:
        gcc_path = Path(toolchain_bin) / gcc_name
        if gcc_path.is_file():
            return str(Path(toolchain_bin).resolve()), toolchain_prefix
        raise FileNotFoundError(f"指定路径中未找到 {gcc_name}: {toolchain_bin}")

    # 2. 检查 PATH
    which = shutil.which(gcc_name)
    if which:
        bin_dir = str(Path(which).parent)
        return bin_dir, toolchain_prefix

    # 3. 搜索已知安装路径
    for search in _TOOLCHAIN_SEARCH_PATHS:
        gcc_path = Path(search) / gcc_name
        if gcc_path.is_file():
            return str(Path(search).resolve()), toolchain_prefix

    raise FileNotFoundError(
        f"未找到 RISC-V GCC 工具链 ({gcc_name})。\n"
        f"请使用 --toolchain-bin 指定路径，或将其加入 PATH。\n"
        f"已搜索的默认路径: {_TOOLCHAIN_SEARCH_PATHS}"
    )


# ===================================================================
# CMake 生成
# ===================================================================

def generate_cmake(config: Dict[str, Any],
                   project_root: Path,
                   toolchain_bin: str,
                   toolchain_prefix: str) -> Path:
    """生成 CMakeLists.txt 和 toolchain_eide.cmake。

    Args:
        config:           parse_eide_config 返回的配置字典。
        project_root:     工程根目录。
        toolchain_bin:    工具链 bin 目录。
        toolchain_prefix: 工具链前缀。

    Returns:
        CMakeLists.txt 所在目录的路径（IDE/CMake）。
    """
    cmake_dir = project_root / "IDE" / "CMake"
    cmake_dir.mkdir(parents=True, exist_ok=True)

    name = config["name"]
    defines = config["defines"]
    inc_dirs = config["inc_dirs"]
    linker_script = config["linker_script"]

    arch = config["arch"]
    abi = config["abi"]
    code_model = config["code_model"]
    opt = config["optimization"]
    debug_flag = config["debug_flag"]
    c_std_flag = config["c_std_flag"]
    misc_flags = config["misc_flags"]
    c_flags_extra = config["c_flags_extra"]
    asm_flags_extra = config["asm_flags_extra"]
    ld_flags = config["ld_flags"]
    lib_flags = config["lib_flags"]

    # --- 扫描源文件 ---
    src_dirs = config["src_dirs"]
    exclude_files = config["exclude_files"]
    c_sources, asm_sources = scan_sources(src_dirs, exclude_files)

    if not c_sources and not asm_sources:
        raise RuntimeError("未扫描到任何源文件，请检查 srcDirs 配置")

    # --- 生成 arch 标志 ---
    arch_flags = f"-march={arch} -mabi={abi} -mcmodel={code_model}"

    # --- 生成 toolchain_eide.cmake ---
    _write_toolchain(cmake_dir, toolchain_bin, toolchain_prefix,
                     arch_flags, opt, debug_flag, c_std_flag,
                     misc_flags, c_flags_extra, asm_flags_extra)

    # --- 生成 CMakeLists.txt ---
    _write_cmake_lists(cmake_dir, project_root, name,
                       inc_dirs, defines, c_sources, asm_sources,
                       arch_flags, opt, debug_flag, c_std_flag,
                       misc_flags, c_flags_extra, asm_flags_extra,
                       linker_script, ld_flags, lib_flags,
                       toolchain_bin, toolchain_prefix)

    return cmake_dir


def _write_toolchain(cmake_dir: Path, gcc_bin: str, gcc_prefix: str,
                     arch_flags: str, opt: str, debug_flag: str,
                     c_std_flag: str, misc_flags: str,
                     c_flags_extra: str, asm_flags_extra: str) -> None:
    """写入 toolchain_eide.cmake。"""
    gcc_bin_fwd = str(Path(gcc_bin)).replace("\\", "/")

    # 公共编译标志
    cmn = f"{arch_flags} -ffunction-sections -fdata-sections -fno-common"

    c_flags = " ".join(x for x in [
        cmn, opt, debug_flag, c_std_flag, "-Wall",
        misc_flags, c_flags_extra
    ] if x)

    asm_flags = " ".join(x for x in [
        arch_flags, "-x assembler-with-cpp", debug_flag,
        misc_flags, asm_flags_extra
    ] if x)

    content = f'''# EIDE Build Skill — 自动生成的 RISC-V GCC 工具链文件
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(GCC_BIN "{gcc_bin_fwd}")
set(GCC_PREFIX "{gcc_prefix}")

set(CMAKE_C_COMPILER   "${{GCC_BIN}}/${{GCC_PREFIX}}gcc.exe")
set(CMAKE_CXX_COMPILER "${{GCC_BIN}}/${{GCC_PREFIX}}g++.exe")
set(CMAKE_ASM_COMPILER "${{GCC_BIN}}/${{GCC_PREFIX}}gcc.exe")

set(CMAKE_C_FLAGS "{c_flags}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "{asm_flags}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "" FORCE)

set(CMAKE_C_LINK_EXECUTABLE
    "<CMAKE_C_COMPILER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
'''

    out = cmake_dir / "toolchain_eide.cmake"
    out.write_text(content, encoding="utf-8")


def _write_cmake_lists(cmake_dir: Path, project_root: Path, name: str,
                       inc_dirs: List[Path], defines: List[str],
                       c_sources: List[Path], asm_sources: List[Path],
                       arch_flags: str, opt: str, debug_flag: str,
                       c_std_flag: str, misc_flags: str,
                       c_flags_extra: str, asm_flags_extra: str,
                       linker_script: str, ld_flags: str, lib_flags: str,
                       toolchain_bin: str, toolchain_prefix: str) -> None:
    """写入 CMakeLists.txt。"""
    proj_root_fwd = str(project_root.resolve()).replace("\\", "/")

    lines: List[str] = []

    lines.append("# EIDE Build Skill — 自动生成的 CMakeLists.txt")
    lines.append("cmake_minimum_required(VERSION 3.21)")
    lines.append(f"project({name} C ASM)")
    lines.append("")
    lines.append(f'set(PROJECT_ROOT "{proj_root_fwd}")')
    lines.append("")

    # --- include 目录 ---
    if inc_dirs:
        lines.append("include_directories(")
        for d in inc_dirs:
            df = str(d).replace("\\", "/")
            lines.append(f'    "{df}"')
        lines.append(")")
        lines.append("")

    # --- 预处理器定义 ---
    if defines:
        lines.append("add_compile_definitions(")
        for d in defines:
            lines.append(f'    {d}')
        lines.append(")")
        lines.append("")

    # --- 源文件 ---
    lines.append(f"add_executable(${{CMAKE_PROJECT_NAME}}")

    for s in asm_sources:
        sf = str(s).replace("\\", "/")
        lines.append(f'    "{sf}"')
    for s in c_sources:
        sf = str(s).replace("\\", "/")
        lines.append(f'    "{sf}"')

    lines.append(")")
    lines.append("")

    # 编译选项已在 toolchain 文件中设置，此处不再重复添加 target_compile_options

    # --- 链接选项 ---
    lines.append(f"target_link_options(${{CMAKE_PROJECT_NAME}} PRIVATE")
    lines.append(f"    {arch_flags}")
    if linker_script:
        ls = str(linker_script).replace("\\", "/")
        lines.append(f'    -T "{ls}"')
    lines.append(")")
    lines.append("")

    # 链接器标志（以空格分隔的独立参数）
    ld_items = [x for x in ld_flags.split() if x]
    if ld_items:
        lines.append(f"target_link_options(${{CMAKE_PROJECT_NAME}} PRIVATE")
        for item in ld_items:
            lines.append(f"    {item}")
        lines.append(")")
        lines.append("")

    # --- 库 ---
    for lib in lib_flags.split():
        if lib.startswith("-l"):
            lib_name = lib[2:]
            lines.append(f"target_link_libraries(${{CMAKE_PROJECT_NAME}} PRIVATE {lib_name})")
    if any(lib.startswith("-l") for lib in lib_flags.split()):
        lines.append("")

    # --- 后处理 ---
    gcc_bin_fwd = str(Path(toolchain_bin)).replace("\\", "/")
    objcopy = f"{gcc_bin_fwd}/{toolchain_prefix}objcopy.exe"
    size_tool = f"{gcc_bin_fwd}/{toolchain_prefix}size.exe"

    lines.append(f"add_custom_command(TARGET ${{CMAKE_PROJECT_NAME}} POST_BUILD")
    lines.append(f'    COMMAND "{objcopy}" -O ihex')
    lines.append(f'            "$<TARGET_FILE:${{CMAKE_PROJECT_NAME}}>"')
    lines.append(f'            "${{CMAKE_PROJECT_NAME}}.hex"')
    lines.append(f'    COMMAND "{objcopy}" -O binary')
    lines.append(f'            "$<TARGET_FILE:${{CMAKE_PROJECT_NAME}}>"')
    lines.append(f'            "${{CMAKE_PROJECT_NAME}}.bin"')
    lines.append(f'    COMMAND "{size_tool}"')
    lines.append(f'            "$<TARGET_FILE:${{CMAKE_PROJECT_NAME}}>"')
    lines.append(f'    COMMENT "Generating .hex, .bin, and size report"')
    lines.append(f")")

    content = "\n".join(lines) + "\n"
    out = cmake_dir / "CMakeLists.txt"
    out.write_text(content, encoding="utf-8")


# ===================================================================
# 编译
# ===================================================================

def run_build(cmake_dir: Path, build_dir: Path, verbose: bool = False) -> int:
    """执行 CMake 配置和编译。

    Args:
        cmake_dir:  CMakeLists.txt 所在目录。
        build_dir:  构建输出目录。
        verbose:    为 True 时打印完整编译输出。

    Returns:
        退出码: 0=成功, 2=CMake 配置失败, 3=编译失败。
    """
    build_dir.mkdir(parents=True, exist_ok=True)

    stdout_dest = None if verbose else subprocess.DEVNULL
    stderr_dest = None if verbose else subprocess.PIPE

    # --- CMake 配置 ---
    ret = subprocess.run(
        [
            "cmake", "-S", str(cmake_dir), "-B", str(build_dir),
            "-G", "Ninja",
            "-DCMAKE_TOOLCHAIN_FILE=toolchain_eide.cmake",
            "-DCMAKE_BUILD_TYPE=Debug",
        ],
        stdout=stdout_dest,
        stderr=stderr_dest,
        cwd=str(cmake_dir),
    )
    if ret.returncode != 0:
        if not verbose and stderr_dest is subprocess.PIPE:
            sys.stderr.write(ret.stderr.decode("utf-8", errors="replace") if ret.stderr else "")
        sys.stderr.write("\nCMake 配置失败\n")
        return 2

    # --- CMake 编译 ---
    ret = subprocess.run(
        ["cmake", "--build", str(build_dir)],
        stdout=stdout_dest,
        stderr=stderr_dest,
    )
    if ret.returncode != 0:
        if not verbose and stderr_dest is subprocess.PIPE:
            sys.stderr.write(ret.stderr.decode("utf-8", errors="replace") if ret.stderr else "")
        sys.stderr.write("\n编译失败\n")
        return 3

    return 0


# ===================================================================
# 辅助函数
# ===================================================================

def _resolve_rel(base_dir: Path, rel: str) -> Path:
    """将相对于 base_dir 的路径解析为绝对路径。"""
    # 标准化路径分隔符
    rel_norm = rel.replace("\\", "/")
    return (base_dir / rel_norm).resolve()


def _resolve_ld_script_in_flags(ld_flags: str, base_dir: Path) -> str:
    """将 LD_FLAGS 中 -T 后面的相对路径解析为绝对路径。"""
    # 匹配 -T "path" 或 -T path
    def _replace_t_path(match):
        path = match.group(1)
        if not Path(path).is_absolute():
            resolved = _resolve_rel(base_dir, path)
            return f'-T "{resolved}"'
        return match.group(0)

    result = re.sub(r'-T\s+"([^"]+)"', _replace_t_path, ld_flags)
    return result


def _extract_ld_script_from_flags(ld_flags: str) -> str:
    """从 LD_FLAGS 中提取链接脚本路径。"""
    m = re.search(r'-T\s+"([^"]+)"', ld_flags)
    if m:
        return m.group(1)
    m = re.search(r'-T\s+(\S+)', ld_flags)
    if m:
        return m.group(1)
    return ""


def _extract_specs_flags(misc_flags: str) -> str:
    """从 misc_flags 中提取 --specs= 标志（链接时需要传递给 gcc 驱动）。"""
    parts = misc_flags.split()
    specs = [p for p in parts if p.startswith("--specs=")]
    return " ".join(specs)


def _strip_ld_script_from_flags(ld_flags: str) -> str:
    """从 LD_FLAGS 中移除 -T 及其路径参数。"""
    # 移除 -T "path" 或 -T path
    result = re.sub(r'-T\s+"[^"]*"', '', ld_flags)
    result = re.sub(r'-T\s+\S+', '', result)
    # 清理多余空格
    result = re.sub(r'\s{2,}', ' ', result)
    return result


# ===================================================================
# 主入口
# ===================================================================

def main() -> int:
    import argparse

    parser = argparse.ArgumentParser(
        description="EIDE Build Skill — 解析 EIDE 配置 → 生成 CMake → 编译"
    )
    parser.add_argument(
        "project_root", nargs="?", default=".",
        help="工程根目录（默认当前目录）"
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true",
        help="输出完整编译日志"
    )
    parser.add_argument(
        "--toolchain-bin",
        help="工具链 bin 目录（覆盖自动检测）"
    )
    parser.add_argument(
        "--toolchain-prefix",
        default="riscv-none-embed-",
        help="工具链前缀（默认 riscv-none-embed-）"
    )
    parser.add_argument(
        "--dry-run", action="store_true",
        help="仅生成 CMake 文件，不执行编译"
    )
    args = parser.parse_args()

    project_root = Path(args.project_root).resolve()
    if not project_root.is_dir():
        sys.stderr.write(f"FATAL: 工程目录不存在: {project_root}\n")
        return 1

    # 1. 查找 EIDE 配置
    try:
        yaml_path = find_eide_config(project_root)
    except FileNotFoundError as exc:
        sys.stderr.write(f"FATAL: {exc}\n")
        return 1

    if args.verbose:
        print(f"[INFO] 找到 EIDE 配置: {yaml_path}")

    # 2. 解析配置
    try:
        config = parse_eide_config(yaml_path)
    except Exception as exc:
        sys.stderr.write(f"FATAL: YAML 解析失败: {exc}\n")
        return 1

    if args.verbose:
        print(f"[INFO] 项目名: {config['name']}")
        print(f"[INFO] 架构: {config['arch']}, ABI: {config['abi']}")
        print(f"[INFO] 源文件目录: {len(config['src_dirs'])} 个")
        print(f"[INFO] 头文件目录: {len(config['inc_dirs'])} 个")

    # 3. 检测工具链
    try:
        gcc_bin, prefix = detect_toolchain(args.toolchain_bin, args.toolchain_prefix)
    except FileNotFoundError as exc:
        sys.stderr.write(f"FATAL: {exc}\n")
        return 1

    if args.verbose:
        print(f"[INFO] 工具链: {gcc_bin}/{prefix}gcc.exe")

    # 4. 生成 CMake 文件
    try:
        cmake_dir = generate_cmake(config, project_root, gcc_bin, prefix)
    except Exception as exc:
        sys.stderr.write(f"FATAL: CMake 生成失败: {exc}\n")
        return 1

    if args.verbose:
        print(f"[INFO] CMake 文件已生成: {cmake_dir}")

    if args.dry_run:
        print("[INFO] dry-run 模式，跳过编译")
        return 0

    # 5. 编译
    build_dir = cmake_dir / "build"
    exit_code = run_build(cmake_dir, build_dir, args.verbose)
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
