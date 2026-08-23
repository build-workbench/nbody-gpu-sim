# nbody-gpu-sim

<div align="center">

**CUDA 加速的百万粒子 N 体引力模拟引擎 · N-Body GPU Simulation**

[![CI](https://github.com/vibe-knight/nbody-gpu-sim/actions/workflows/ci.yml/badge.svg)](https://github.com/vibe-knight/nbody-gpu-sim/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)](CMakeLists.txt)
[![CUDA](https://img.shields.io/badge/CUDA-11.8+-76B900?logo=nvidia&logoColor=white)](CMakeLists.txt)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-blue)]()
[![Docs](https://img.shields.io/badge/Docs-GitHub%20Pages-4f46e5)](https://vibe-knight.github.io/nbody-gpu-sim/)

</div>

---

> **一句话简介**：一个 CUDA 加速的百万粒子 N 体引力模拟引擎，支持 **Direct N² / Barnes-Hut / Spatial Hash** 三种力计算算法实时切换，配合 OpenGL 零拷贝实时可视化，内置基准测试与状态导入导出。

---

## ✨ 项目亮点

- **百万粒子级实时模拟** — CUDA 并行、SoA 内存布局，百万粒子仍可交互
- **三种力算法，运行时热切换** — 按 `1` / `2` / `3` 即可在 Direct N²、Barnes-Hut、Spatial Hash 之间切换
- **Velocity Verlet 辛积分** — 长时间模拟保持能量稳定
- **CUDA-OpenGL 零拷贝渲染** — `cuda-graphics` 互操作，GPU 内存直接可视化，无 CPU↔GPU 拷贝
- **内置性能基准** — 非交互 benchmark 模式，JSON 结构化输出，方便复现与对比
- **状态导入 / 导出** — 支持 checkpoint 序列化，可复现特定初始条件
- **工程严谨性** — RAII 资源管理、属性测试、质量守恒校验、三层测试体系

## 🚀 快速开始

### 环境要求

| 组件 | 最低要求 |
|------|---------|
| NVIDIA GPU | Compute Capability 7.5+（RTX 20 系列起） |
| CUDA Toolkit | 11.8+ |
| CMake | 3.18+ |
| 编译器 | GCC 9+ / Clang 10+（C++17） |
| 图形 | OpenGL 3.3+ / GLFW 3.3+ / GLEW 2.1+ / GLM 0.9.9+ |

### 构建与运行

```bash
git clone https://github.com/vibe-knight/nbody-gpu-sim.git
cd nbody-gpu-sim

./scripts/build.sh
./build/nbody_sim 100000
```

启动后即可看到 10 万粒子绕共同质心旋转的实时模拟，窗口标题实时显示粒子数与 FPS。

### 测试与基准

```bash
./scripts/test.sh          # 单元测试 + 属性测试
./scripts/benchmark.sh     # 输出 JSON 格式的性能报告
```

## 🎮 操作

| 按键 | 功能 |
|------|------|
| `Space` | 暂停 / 恢复 |
| `R` | 重置模拟 |
| `1` / `2` / `3` | 切换力算法：Direct N² / Barnes-Hut / Spatial Hash |
| `C` | 重置相机 |
| `F1` | 切换诊断面板（需启用 UI 构建） |
| 鼠标拖拽 | 旋转视角 |
| 滚轮 | 缩放 |
| `Esc` | 退出 |

## 🧮 算法

| 算法 | 时间复杂度 | 适用场景 |
|------|-----------|---------|
| **Direct N²** | O(N²) | 小规模系统（≤1 万粒子）、作为精确参考基线 |
| **Barnes-Hut** | O(N log N) | 大规模引力系统（10 万+ 粒子），远距离粒子分组近似 |
| **Spatial Hash** | O(N) | 短程作用力（分子动力学、粒子流体），3×3×3 邻域搜索 |

三种算法均以 CPU 参考实现做差分验证，确保数值一致性。

## 📊 性能

| 粒子数 | Direct N² | Barnes-Hut | Spatial Hash |
|--------|-----------|------------|--------------|
| 1 万 | 实时 | 实时 | 实时 |
| 10 万 | 慢 | 实时 | 实时 |
| 100 万 | 不可用 | 可交互 | 实时 |

*基准测试环境：NVIDIA RTX 3080。可在你自己的硬件上运行 `./scripts/benchmark.sh` 复现。*

## ⚙️ CLI 参数

```text
nbody_sim [选项]

  --particles N          粒子数量（默认 10000）
  --method NAME          力算法：direct-n2 | barnes-hut | spatial-hash
  --dt VALUE             积分时间步长（默认 0.001）
  --gravity VALUE        引力常数 G（默认 1.0）
  --softening VALUE      软化参数（默认 0.1）
  --theta VALUE          Barnes-Hut 展开阈值 θ（默认 0.5）
  --cell-size VALUE      空间哈希网格尺寸（默认 2.0）
  --cutoff VALUE         空间哈希截断半径（默认 2.0）
  --benchmark            运行非交互基准测试并退出
  --benchmark-steps N    基准测试更新步数（默认 120）
  --benchmark-output P   基准测试 JSON 输出路径
  --export PATH          导出粒子状态到 checkpoint 文件
  --import PATH          从 checkpoint 文件导入粒子状态
  --help                 显示帮助
```

示例：

```bash
# 10 万粒子，Barnes-Hut 算法
./build/nbody_sim --particles 100000 --method barnes-hut

# 跑 500 步基准并输出 JSON
./build/nbody_sim --particles 1000000 --method spatial-hash --benchmark --benchmark-steps 500 --benchmark-output result.json

# 导入初始状态继续模拟
./build/nbody_sim --import checkpoint.bin --particles 50000
```

## 🏗️ 项目结构

```text
nbody-gpu-sim/
├── include/nbody/     # 公共头文件（类型、算法、渲染、工具接口）
├── src/
│   ├── core/          # 粒子系统、CLI 解析
│   ├── cuda/          # 力计算内核、积分器、CUDA-GL 互操作
│   ├── render/        # OpenGL 渲染器、相机、UI 面板
│   └── utils/         # 错误处理、序列化、性能观测
├── tests/             # 单元测试 + 属性测试（三层：core / cuda / render）
├── examples/          # 可运行示例程序
├── benchmarks/        # 基准测试入口
├── scripts/           # 构建 / 测试 / 格式化脚本
├── site/              # GitHub Pages 展示站点（VitePress）
└── CMakeLists.txt     # 主构建配置
```

## 🔧 技术架构

| 模块 | 说明 |
|------|------|
| **ParticleData** | SoA 布局，13 floats/粒子（52 字节），合并访存 |
| **ForceCalculator** | 三种力算法统一接口，CPU 参考实现做差分验证 |
| **Integrator** | Velocity Verlet 辛积分器，保证长期能量稳定 |
| **CudaGLInterop** | CUDA-OpenGL 互操作，渲染零拷贝 |
| **PerformanceObservability** | 阶段计时与性能观测，供 benchmark 输出 |

## 🧪 测试

项目采用三层测试体系：

| 层级 | 覆盖范围 | 运行条件 |
|------|---------|---------|
| `nbody_core_tests` | CLI、序列化、校验、观测 | 无需 GPU，CI 可跑 |
| `nbody_tests` | 粒子数据、力内核、积分器、Barnes-Hut、Spatial Hash | 需要 CUDA GPU |
| `nbody_render_tests` | 相机、颜色映射 | 需要 CUDA + 渲染环境 |

```bash
./scripts/test.sh
```

## 🤝 贡献

欢迎提交 Issue 与 Pull Request。请先阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。

## 📄 License

[MIT](LICENSE) © Vibe Knight
