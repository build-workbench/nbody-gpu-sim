# N-Body 粒子仿真系统

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/CUDA-11.0+-76B900?logo=nvidia&logoColor=white" alt="CUDA">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white" alt="C++">
  <img src="https://img.shields.io/badge/OpenGL-3.3+-5586A4?logo=opengl&logoColor=white" alt="OpenGL">
  <img src="https://img.shields.io/badge/CMake-3.18+-064F8C?logo=cmake&logoColor=white" alt="CMake">
</p>

<p align="center">
  <b>高性能 GPU 加速 N-Body 仿真系统，支持实时可视化</b>
</p>

<p align="center">
  <a href="https://github.com/LessUp/n-body/releases"><img src="https://img.shields.io/github/v/release/LessUp/n-body?include_prereleases" alt="Latest Release"></a>
  <a href="https://github.com/LessUp/n-body/issues"><img src="https://img.shields.io/github/issues/LessUp/n-body" alt="Issues"></a>
  <img src="https://img.shields.io/github/stars/LessUp/n-body?style=social" alt="Stars">
</p>

<p align="center">
  <a href="README.md">English</a> | 简体中文
</p>

---

## ✨ 特性

- 🚀 **高性能 GPU 计算** — CUDA 并行计算，百万粒子实时仿真
- 🔬 **多种力算法** — Direct N²、Barnes-Hut O(N log N)、Spatial Hash O(N)
- 🎨 **实时可视化** — CUDA-OpenGL 互操作，无 CPU-GPU 数据传输
- ⚛️ **能量守恒** — Velocity Verlet 辛积分器
- 🎮 **交互控制** — 相机旋转/缩放、运行时算法切换
- 🧪 **全面测试** — Google Test + RapidCheck 基于属性的测试
- 📦 **易于构建** — 基于 CMake，跨平台

---

## 🚀 快速开始

### 前置要求

| 组件 | 最低配置 | 推荐配置 |
|------|----------|----------|
| GPU | NVIDIA CC 7.0+ | NVIDIA CC 8.0+ |
| CUDA | 11.0 | 12.0+ |
| CMake | 3.18 | 3.25+ |
| OpenGL | 3.3 | 4.5+ |

### 安装 (Ubuntu)

```bash
# 安装依赖
sudo apt-get install -y cmake libglfw3-dev libglew-dev libglm-dev

# 克隆仓库
git clone https://github.com/LessUp/n-body.git
cd n-body

# 构建
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# 运行
./nbody_sim 100000    # 10万粒子
./nbody_sim 1000000   # 100万粒子！
```

### Windows

Windows 构建说明请参见 [快速入门指南](docs/zh-CN/getting-started.md)。

---

## 📊 性能

NVIDIA RTX 3080 测试：

| 粒子数 | Direct N² | Barnes-Hut | Spatial Hash |
|--------|-----------|------------|--------------|
| 1万 | 60 FPS | 120+ FPS | 120+ FPS |
| 10万 | ~10 FPS | 60+ FPS | 90+ FPS |
| 100万 | — | 25+ FPS | 60+ FPS |

**内存使用**: 每粒子 ~52 字节 + 算法开销

---

## 🎮 控制

| 按键 | 功能 |
|------|------|
| `Space` | ⏯️ 暂停/继续 |
| `R` | 🔄 重置仿真 |
| `1/2/3` | 🔀 切换算法 |
| `C` | 📷 重置相机 |
| `Esc` | ❌ 退出 |
| `鼠标拖动` | 🔄 旋转视角 |
| `滚轮` | 🔍 缩放 |

---

## 📚 文档

### 入门指南

- [快速入门指南](docs/zh-CN/getting-started.md) — 完整的安装和使用
- [示例](examples/) — 常见用例的代码示例

### 参考

- [架构概览](docs/zh-CN/architecture.md) — 系统设计和组件
- [算法详解](docs/zh-CN/algorithms.md) — 算法说明
- [API 参考](docs/zh-CN/api.md) — 完整的 API 文档
- [性能指南](docs/zh-CN/performance.md) — 优化策略

### 🌐 语言版本

| 语言 | 文档 |
|------|------|
| 🇺🇸 English | [Docs](./docs/en/) |
| 🇨🇳 简体中文 | [文档](./docs/zh-CN/) |

---

## 🏗️ 架构

```
┌─────────────────────────────────────────┐
│  应用层 (GLFW, 输入, 状态)              │
├─────────────────────────────────────────┤
│  仿真层 (ParticleSystem, 力计算)        │
├─────────────────────────────────────────┤
│  渲染层 (OpenGL, 相机)                  │
├─────────────────────────────────────────┤
│  GPU 内存 (CUDA, 共享 VBO)              │
└─────────────────────────────────────────┘
```

详情参见 [架构文档](docs/zh-CN/architecture.md)。

---

## 🧪 测试

```bash
cd build
./nbody_tests

# 运行特定测试套件
./nbody_tests --gtest_filter=ForceCalculation.*
```

---

## 💡 使用示例

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 100000;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;
    
    ParticleSystem system;
    system.initialize(config);
    
    for (int i = 0; i < 1000; ++i) {
        system.update(system.getTimeStep());
    }
    
    system.saveState("checkpoint.nbody");
    return 0;
}
```

---

## 🔬 算法

| 算法 | 复杂度 | 适用场景 |
|------|--------|----------|
| Direct N² | O(N²) | 小规模系统、精度测试 |
| Barnes-Hut | O(N log N) | 大规模引力仿真 |
| Spatial Hash | O(N) | 短程力仿真 |

详情参见 [算法文档](docs/zh-CN/algorithms.md)。

---

## 🤝 贡献

贡献指南请参见 [CONTRIBUTING.md](CONTRIBUTING.md)。

---

## 📄 许可证

MIT 许可证 — 详见 [LICENSE](LICENSE)。

---

## 🌟 Star 历史

[![Star History Chart](https://api.star-history.com/svg?repos=LessUp/n-body&type=Date)](https://star-history.com/#LessUp/n-body&Date)

---

## 📖 引用

如果在研究中使用本项目，请引用：

```bibtex
@software{nbody_simulation,
  title = {N-Body Particle Simulation System},
  author = {N-Body Simulation Team},
  url = {https://github.com/LessUp/n-body},
  year = {2025}
}
```

---

## 相关项目

- [Barnes & Hut (1986)](https://doi.org/10.1038/324446a0) — 原始 Barnes-Hut 算法
- [GPU Gems 3](https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-31-fast-n-body-simulation-cuda) — CUDA N-Body 仿真
- [NVIDIA CUDA Samples](https://github.com/NVIDIA/cuda-samples) — 官方 CUDA 示例
