"""Exercise 4: Time vs number of processors and speedup for
Exercise 2 (sum 1..10,000,000) and Exercise 3 (Monte Carlo Pi).

Compiles both programs, runs each with 1..12 processes (REPEATS times,
median taken), saves results.csv and plots time.png and speedup.png.

Usage (from the repo root):  python Exercise4/benchmark.py
"""
import csv
import re
import statistics
import subprocess
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parent.parent
OUT = Path(__file__).resolve().parent
PROCS = [1, 2, 4, 6, 8, 10, 12]
REPEATS = 5

PROGRAMS = {
    "Sum 1..10,000,000 (Ex 2)": ROOT / "Exercise2" / "sum",
    "Monte Carlo Pi (Ex 3)": ROOT / "Exercise3" / "pi",
}
COLORS = ["#2a78d6", "#eb6834"]


# run from the program's folder with relative names: the mpicc wrapper
# does not handle spaces in absolute paths
def build(base):
    subprocess.run(["mpicc", "-O2", "-o", f"{base.name}.exe", f"{base.name}.c"],
                   cwd=base.parent, check=True)


def run(base, n):
    out = subprocess.run(["mpiexec", "-n", str(n), f"{base.name}.exe"],
                         cwd=base.parent, capture_output=True, text=True,
                         check=True).stdout
    return float(re.search(r"Time:\s*([0-9.]+)", out).group(1))


results = {}
for name, base in PROGRAMS.items():
    build(base)
    results[name] = {}
    for n in PROCS:
        times = [run(base, n) for _ in range(REPEATS)]
        results[name][n] = statistics.median(times)
        print(f"{name:28s} n={n:2d}  time={results[name][n]:.6f} s  "
              f"speedup={results[name][1] / results[name][n]:.2f}")

with open(OUT / "results.csv", "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["program", "processes", "time_s", "speedup"])
    for name, row in results.items():
        for n, t in row.items():
            w.writerow([name, n, f"{t:.6f}", f"{row[1] / t:.3f}"])

plt.rcParams.update({
    "font.size": 10, "axes.spines.top": False, "axes.spines.right": False,
    "axes.edgecolor": "#8a8a85", "axes.labelcolor": "#3d3d3a",
    "xtick.color": "#5c5c58", "ytick.color": "#5c5c58",
    "axes.grid": True, "grid.color": "#e6e5e0", "grid.linewidth": 0.8,
    "figure.facecolor": "#fcfcfb", "axes.facecolor": "#fcfcfb",
})

# Time vs processors: one panel per program (different time scales)
fig, axes = plt.subplots(1, 2, figsize=(10, 4))
for ax, (name, row), color in zip(axes, results.items(), COLORS):
    ms = [row[n] * 1000 for n in PROCS]
    ax.plot(PROCS, ms, color=color, linewidth=2, marker="o", markersize=7,
            markeredgecolor="#fcfcfb", markeredgewidth=2)
    ax.set_title(name, loc="left", fontsize=11, color="#1f1f1d")
    ax.set_xlabel("Number of processes")
    ax.set_ylabel("Time (ms)")
    ax.set_xticks(PROCS)
    ax.set_ylim(0, max(ms) * 1.12)
    ax.annotate(f"{ms[0]:.1f} ms", (PROCS[0], ms[0]), textcoords="offset points",
                xytext=(10, -4), va="top", color="#3d3d3a")
    ax.annotate(f"{ms[-1]:.1f} ms", (PROCS[-1], ms[-1]), textcoords="offset points",
                xytext=(0, 12), ha="center", color="#3d3d3a")
fig.suptitle("Execution time vs number of processes", x=0.01, ha="left",
             fontsize=13, color="#1f1f1d")
fig.tight_layout()
fig.savefig(OUT / "time.png", dpi=150)

# Speedup = T(1) / T(p)
fig, ax = plt.subplots(figsize=(7, 4.5))
ax.plot(PROCS, PROCS, color="#8a8a85", linewidth=1.5, linestyle="--")
ax.annotate("Ideal", (PROCS[-1], PROCS[-1]), textcoords="offset points",
            xytext=(6, 0), va="center", color="#5c5c58")
for (name, row), color in zip(results.items(), COLORS):
    sp = [row[1] / row[n] for n in PROCS]
    ax.plot(PROCS, sp, color=color, linewidth=2, marker="o", markersize=7,
            markeredgecolor="#fcfcfb", markeredgewidth=2, label=name)
    ax.annotate(f"{sp[-1]:.1f}x", (PROCS[-1], sp[-1]), textcoords="offset points",
                xytext=(8, -3), color="#3d3d3a")
ax.set_title("Speedup vs number of processes", loc="left", fontsize=13, color="#1f1f1d")
ax.set_xlabel("Number of processes")
ax.set_ylabel("Speedup  T(1) / T(p)")
ax.set_xticks(PROCS)
ax.set_xlim(0.5, PROCS[-1] + 2.5)
ax.set_ylim(bottom=0)
ax.legend(frameon=False, loc="upper left")
fig.tight_layout()
fig.savefig(OUT / "speedup.png", dpi=150)
print("Saved results.csv, time.png, speedup.png in", OUT)
