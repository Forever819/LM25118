#!/usr/bin/env python3
"""
LM25118 数控电源 — 自动效率测试上位机

解析串口 EFF 数据，每次测试自动导出时间戳 CSV 并绘制效率曲线。

用法:
  python efficiency_plotter.py              # 自动检测 COM
  python efficiency_plotter.py COM3         # 指定端口
  python efficiency_plotter.py data.txt     # 从文件读取

数据格式 (MCU → PC):
  EFF_START
  EFF,<Vout>,<eff>
  EFF,12.00,85.3
  ...
  EFF,<Vout>,T/O         ← 超时跳点 (不写入 CSV)
  EFF_END
  EFF_ABORT
"""

import csv
import os
import re
import sys
from datetime import datetime
from dataclasses import dataclass, field
from typing import List, Optional

import matplotlib
import matplotlib.pyplot as plt

_has_fig = False  # set True by plot_efficiency, read by live_monitor to pump GUI


# ── 数据模型 ──────────────────────────────────────────────────────

@dataclass
class EffData:
    vout: float
    eff: float

@dataclass
class EffResult:
    points: List[EffData] = field(default_factory=list)
    skipped: List[float] = field(default_factory=list)  # T/O Vout
    aborted: bool = False
    complete: bool = False

    @property
    def max_eff_point(self) -> Optional[EffData]:
        return max(self.points, key=lambda p: p.eff) if self.points else None


# ── 解析器 ────────────────────────────────────────────────────────

EFF_RE = re.compile(r"^EFF,(\d+\.?\d*),(\d+\.?\d*)")
EFF_TO_RE = re.compile(r"^EFF,(\d+\.?\d*),T/O")

class EffParser:
    def __init__(self):
        self.result = EffResult()

    def feed_line(self, line: str) -> bool:
        """返回 True = 测试结束."""
        line = line.strip()
        if not line:
            return False
        if line == "EFF_START":
            self.result = EffResult()
            return False
        if line == "EFF_END":
            self.result.complete = True
            return True
        if line == "EFF_ABORT":
            self.result.aborted = True
            return True
        m = EFF_TO_RE.match(line)
        if m:
            self.result.skipped.append(float(m.group(1)))
            return False
        m = EFF_RE.match(line)
        if m:
            self.result.points.append(EffData(
                vout=float(m.group(1)), eff=float(m.group(2))))
        return False


# ── CSV 导出 ──────────────────────────────────────────────────────

def save_csv(result: EffResult, output_dir: str = ".") -> str:
    """导出 CSV，返回文件路径。"""
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = os.path.join(output_dir, f"eff_test_{ts}.csv")
    points = sorted(result.points, key=lambda p: p.vout)
    with open(filename, "w", newline="", encoding="utf-8-sig") as f:
        w = csv.writer(f)
        w.writerow(["Vout(V)", "Efficiency(%)"])
        for p in points:
            w.writerow([f"{p.vout:.2f}", f"{p.eff:.1f}"])
        if result.skipped:
            w.writerow([])
            w.writerow(["# Skipped", f"{len(result.skipped)} points"])
        if result.aborted:
            w.writerow(["# ABORTED"])
    return filename


# ── 绘图 ──────────────────────────────────────────────────────────

def plot_efficiency(result: EffResult):
    """绘制效率曲线，关闭旧图窗，显示新图窗。"""
    if not result.points:
        return

    points = sorted(result.points, key=lambda p: p.vout)
    vouts = [p.vout for p in points]
    effs = [p.eff for p in points]

    # 中文字体
    for fn in ["Microsoft YaHei", "SimHei", "Noto Sans CJK SC", "Arial Unicode MS"]:
        try:
            matplotlib.font_manager.findfont(fn, fallback_to_default=False)
            plt.rcParams["font.sans-serif"] = [fn]
            plt.rcParams["axes.unicode_minus"] = False
            break
        except Exception:
            continue

    # Y 轴自动缩放
    eff_min = min(effs)
    eff_max = max(effs)
    pad = max((eff_max - eff_min) * 0.15, 3.0)
    y_low = max(0, float(int((eff_min - pad) / 5) * 5))
    y_high = min(100, float(int((eff_max + pad + 4.99) / 5) * 5))

    # 关闭旧图窗
    plt.close("all")

    fig, ax = plt.subplots(figsize=(10, 6))

    # 居中到主显示器
    try:
        import tkinter as tk
        _root = tk.Tk()
        sw = _root.winfo_screenwidth()
        sh = _root.winfo_screenheight()
        _root.destroy()
        fig_w, fig_h = fig.get_size_inches() * fig.dpi
        x = max(0, int((sw - fig_w) // 2))
        y = max(0, int((sh - fig_h) // 2))
        fig.canvas.manager.window.wm_geometry(f"+{x}+{y}")
    except Exception:
        pass
    ax.plot(vouts, effs, "b-o", markersize=4, linewidth=1.5, label="Efficiency")

    # 峰值
    mp = result.max_eff_point
    if mp:
        ax.plot(mp.vout, mp.eff, "r*", markersize=15, zorder=5)
        ax.annotate(f"Peak: {mp.eff:.1f}% @ {mp.vout:.1f}V",
                    xy=(mp.vout, mp.eff),
                    xytext=(mp.vout + 3, mp.eff - 2),
                    fontsize=11, color="red",
                    arrowprops=dict(arrowstyle="->", color="red"),
                    bbox=dict(boxstyle="round,pad=0.3", fc="yellow", alpha=0.7))

    # 跳点标记
    if result.skipped:
        ax.scatter(result.skipped, [y_low + 2] * len(result.skipped),
                   marker="x", color="gray", s=40, zorder=3,
                   label=f"Skipped ({len(result.skipped)})")
        ax.legend(fontsize=10)

    ax.set_xlabel("Output Voltage (V)", fontsize=12)
    ax.set_ylabel("Efficiency (%)", fontsize=12)
    ax.set_title("Efficiency Test", fontsize=14)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, max(vouts) + 2)
    ax.set_ylim(y_low, y_high)
    ax.yaxis.set_major_locator(plt.MultipleLocator(5))
    ax.yaxis.set_minor_locator(plt.MultipleLocator(1))

    status = f"Points: {len(points)} | {min(vouts):.1f}V–{max(vouts):.1f}V"
    if mp:
        status += f" | Peak: {mp.eff:.1f}%"
    if result.aborted:
        status += " | ABORTED"
    fig.text(0.5, 0.01, status, ha="center", fontsize=10,
             style="italic", alpha=0.7)
    plt.tight_layout(rect=[0, 0.03, 1, 1])
    plt.show(block=False)
    plt.pause(0.5)
    global _has_fig
    _has_fig = True  # signal live_monitor to pump GUI events


# ── 实时监听 ──────────────────────────────────────────────────────

def live_monitor(port: str, baud: int = 115200):
    try:
        import serial
    except ImportError:
        print("[Error] pyserial required: pip install pyserial"); sys.exit(1)

    test_count = 0
    ser = serial.Serial(port, baud, timeout=0.5)
    print(f"[Live] Listening on {port} @ {baud} baud...")
    print("[Live] Press Ctrl+C to stop.\n")

    parser = EffParser()
    global _has_fig
    _has_fig = False

    try:
        while True:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if not line:
                if _has_fig:
                    plt.pause(0.05)  # pump GUI to keep window responsive
                else:
                    import time
                    time.sleep(0.02)
                continue

            print(f"  < {line}")
            done = parser.feed_line(line)

            if done:
                test_count += 1
                pts = len(parser.result.points)
                if pts:
                    csv_path = save_csv(parser.result)
                    print(f"  → Test #{test_count}: {pts} pts → {csv_path}")
                    plot_efficiency(parser.result)
                else:
                    print(f"  → Test #{test_count}: no data")
                parser = EffParser()
                plt.pause(0.5)
    except KeyboardInterrupt:
        print("\n[Live] Stopped. Close plot window to exit.")
    finally:
        ser.close()
        plt.ioff()
        plt.show()


# ── 文件模式 ──────────────────────────────────────────────────────

def read_from_file(filepath: str):
    parser = EffParser()
    with open(filepath, "r", encoding="utf-8") as f:
        for line in f:
            parser.feed_line(line.strip())

    if parser.result.points:
        csv_path = save_csv(parser.result)
        print(f"[File] {len(parser.result.points)} pts")
        if parser.result.skipped:
            print(f"[File] {len(parser.result.skipped)} skipped")
        print(f"[File] → {csv_path}")
        plot_efficiency(parser.result)
        plt.show()
    else:
        print(f"[Error] No valid data in {filepath}")


def auto_detect_port() -> Optional[str]:
    try:
        import serial.tools.list_ports
        ports = serial.tools.list_ports.comports()
        return ports[0].device if ports else None
    except ImportError:
        return None

def main():
    args = sys.argv[1:]
    # optional --title for terminal window title
    extra = {}
    for i, a in enumerate(args):
        if a == "--title" and i + 1 < len(args):
            extra["title"] = args[i + 1]
            # remove both from args
            args = args[:i] + args[i + 2:]
            break
    if "title" in extra:
        try:
            import os
            os.system(f"title {extra['title']}")
        except Exception:
            pass

    if not args:
        port = auto_detect_port()
        if port:
            print(f"[Auto] Detected: {port}")
            live_monitor(port)
        else:
            print("Usage: python efficiency_plotter.py [COMx|file.txt]")
            sys.exit(1)
    else:
        arg = args[0]
        if any(arg.lower().endswith(e) for e in [".txt", ".log", ".csv", ".dat"]):
            read_from_file(arg)
        else:
            live_monitor(arg)

if __name__ == "__main__":
    main()
