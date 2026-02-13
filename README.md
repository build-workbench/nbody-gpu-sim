# N-Body Particle Simulation System

超大规模 N-Body 粒子仿真系统，支持百万级粒子的 GPU 并行计算和实时可视化。

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![CUDA](https://img.shields.io/badge/CUDA-11.0+-76B900?logo=nvidia&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3+-5586A4?logo=opengl&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.18+-064F8C?logo=cmake&logoColor=white)

## 特性

- **高性能 GPU 计算**: 利用 CUDA 并行计算，支持百万级粒子实时仿真
- **多种力计算算法**:
  - Direct N² - O(N²) 精确计算
  - Barnes-Hut - O(N log N) 树算法加速
  - Spatial Hash - O(N) 空间哈希（短程力）
- **零拷贝渲染**: CUDA-OpenGL 互操作，无 CPU-GPU 数据传输
- **Velocity Verlet 积分**: 辛积分器，保证能量守恒
- **实时交互**: 相机控制、参数调节、算法切换

## 系统要求

### 硬件
- NVIDIA GPU (Compute Capability 7.5+)
- 至少 4GB 显存（百万粒子）

### 软件
- CUDA Toolkit 11.0+
- CMake 3.18+
- OpenGL 3.3+
- GLFW 3.3+
- GLEW
- GLM

### 操作系统
- Linux (推荐 Ubuntu 20.04+)
- Windows 10+ (需要 Visual Studio 2019+)

## 快速开始

### 安装依赖 (Ubuntu)

```bash
# CUDA Toolkit
# 从 https://developer.nvidia.com/cuda-downloads 下载安装

# 其他依赖
sudo apt-get update
sudo apt-get install -y cmake libglfw3-dev libglew-dev libglm-dev
```

### 构建

```bash
git clone <repository-url>
cd n-body

mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 运行

```bash
# 默认 10000 粒子
./nbody_sim

# 指定粒子数量
./nbody_sim 100000

# 百万粒子
./nbody_sim 1000000
```

## 使用指南

### 键盘控制

| 按键 | 功能 |
|------|------|
| `Space` | 暂停/继续仿真 |
| `R` | 重置仿真 |
| `1` | 切换到 Direct N² 算法 |
| `2` | 切换到 Barnes-Hut 算法 |
| `3` | 切换到 Spatial Hash 算法 |
| `C` | 重置相机位置 |
| `Esc` | 退出程序 |

### 鼠标控制

| 操作 | 功能 |
|------|------|
| 左键拖动 | 旋转视角 |
| 滚轮 | 缩放 |

## 项目结构

```
n-body/
├── CMakeLists.txt              # CMake 构建配置
├── README.md                   # 项目说明
├── docs/                       # 详细文档
│   ├── API.md                  # API 参考
│   ├── ALGORITHMS.md           # 算法说明
│   └── PERFORMANCE.md          # 性能优化指南
├── include/nbody/              # 头文件
│   ├── types.hpp               # 类型定义
│   ├── particle_data.hpp       # 粒子数据管理
│   ├── force_calculator.hpp    # 力计算接口
│   ├── integrator.hpp          # 积分器
│   ├── barnes_hut_tree.hpp     # Barnes-Hut 树
│   ├── spatial_hash_grid.hpp   # 空间哈希网格
│   ├── cuda_gl_interop.hpp     # CUDA-GL 互操作
│   ├── renderer.hpp            # 渲染器
│   ├── camera.hpp              # 相机控制
│   ├── particle_system.hpp     # 粒子系统
│   ├── serialization.hpp       # 序列化
│   └── error_handling.hpp      # 错误处理
├── src/                        # 源代码
│   ├── cuda/                   # CUDA 核函数
│   ├── core/                   # 核心逻辑
│   ├── render/                 # 渲染相关
│   ├── utils/                  # 工具函数
│   └── main.cpp                # 主程序
├── tests/                      # 测试代码
├── shaders/                    # GLSL 着色器
└── .kiro/specs/                # 设计规范
```

## 算法概述

### Direct N² 算法
直接计算所有粒子对之间的引力，时间复杂度 O(N²)。使用 CUDA Shared Memory 优化内存访问。

### Barnes-Hut 算法
使用八叉树将远距离粒子群近似为单个质点，时间复杂度 O(N log N)。通过 θ 参数控制精度与性能的平衡。

### Spatial Hash 算法
将空间划分为网格，只计算相邻格子内粒子的相互作用，时间复杂度 O(N)。适用于短程力（如分子动力学）。

## 性能参考

| 粒子数 | Direct N² | Barnes-Hut | Spatial Hash |
|--------|-----------|------------|--------------|
| 10K    | 60+ FPS   | 60+ FPS    | 60+ FPS      |
| 100K   | ~10 FPS   | 60+ FPS    | 60+ FPS      |
| 1M     | <1 FPS    | ~30 FPS    | 60+ FPS      |

*测试环境: NVIDIA RTX 3080, CUDA 11.8*

## 测试

```bash
cd build
make nbody_tests
./nbody_tests
```

测试包括：
- 单元测试：验证各组件功能
- 属性测试：使用 RapidCheck 验证正确性属性

## API 示例

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    // 配置仿真参数
    SimulationConfig config;
    config.particle_count = 100000;
    config.init_distribution = InitDistribution::SPHERICAL;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;
    config.G = 1.0f;
    
    // 创建粒子系统
    ParticleSystem system;
    system.initialize(config);
    
    // 仿真循环
    while (running) {
        system.update(config.dt);
        // 渲染...
    }
    
    // 保存状态
    system.saveState("simulation.nbody");
    
    return 0;
}
```

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

## 参考文献

1. Barnes, J., & Hut, P. (1986). A hierarchical O(N log N) force-calculation algorithm. *Nature*, 324(6096), 446-449.
2. Nyland, L., Harris, M., & Prins, J. (2007). Fast N-body simulation with CUDA. *GPU Gems 3*, 677-695.
3. Green, S. (2010). Particle simulation using CUDA. *NVIDIA Whitepaper*.
