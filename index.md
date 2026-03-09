---
layout: default
title: N-Body Particle Simulation
---

# N-Body Particle Simulation System

超大规模 N-Body 粒子仿真系统，支持百万级粒子的 GPU 并行计算和实时可视化。

## 核心特性

- **高性能 GPU 计算** — CUDA 并行计算，支持百万级粒子实时仿真
- **多种力计算算法**:
  - Direct N² — O(N²) 精确计算
  - Barnes-Hut — O(N log N) 树算法加速
  - Spatial Hash — O(N) 空间哈希（短程力）
- **零拷贝渲染** — CUDA-OpenGL 互操作，无 CPU-GPU 数据传输
- **Velocity Verlet 积分** — 辛积分器，保证能量守恒
- **实时交互** — 相机控制、参数调节、算法切换

## 文档

- [README](README.md) — 项目概述与快速开始
- [算法详解](docs/ALGORITHMS.md) — Direct N²、Barnes-Hut、Spatial Hash 原理
- [API 参考](docs/API.md) — 接口文档
- [性能分析](docs/PERFORMANCE.md) — 基准测试与优化

## 快速开始

```bash
# 安装依赖 (Ubuntu)
sudo apt-get install -y cmake libglfw3-dev libglew-dev libglm-dev

# 构建
mkdir build && cd build
cmake ..
make -j$(nproc)

# 运行 (默认 10000 粒子)
./nbody_sim

# 百万粒子
./nbody_sim 1000000
```

## 键盘控制

| 按键 | 功能 |
|------|------|
| `1` / `2` / `3` | 切换 Direct / Barnes-Hut / Spatial Hash |
| `WASD` | 移动相机 |
| `Space` / `Shift` | 升降相机 |
| `P` | 暂停/继续 |
| `R` | 重置 |

## 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | CUDA C++17 |
| 渲染 | OpenGL 3.3+, GLFW, GLEW |
| 构建 | CMake 3.18+ |
| GPU | SM 75+ (Turing → Hopper) |

## 链接

- [GitHub 仓库](https://github.com/LessUp/n-body)
- [README](README.md)
