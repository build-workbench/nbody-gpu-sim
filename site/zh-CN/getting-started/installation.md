# 安装

本指南介绍如何在您的系统上安装和构建 N-Body 模拟器。

## 系统要求

### 硬件

- **GPU**: NVIDIA GPU，CUDA 计算能力 3.0+
- **内存**: 最少 8 GB，推荐 16 GB（用于 100 万粒子）
- **存储**: 约 500 MB 构建文件

### 软件

| 依赖 | 版本 | 说明 |
|------|------|------|
| CUDA Toolkit | 11.0+ | GPU 加速必需 |
| CMake | 3.18+ | 构建系统 |
| C++ 编译器 | C++20 | GCC 10+, Clang 12+, MSVC 19.28+ |
| OpenGL | 3.3+ | 可视化 |
| GLFW | 3.3+ | 窗口管理 |
| GLEW | 2.1+ | OpenGL 扩展 |
| GLM | 0.9.9+ | 数学库 |

## Ubuntu/Debian 安装

```bash
# CUDA（按照 NVIDIA 指南安装）
# https://developer.nvidia.com/cuda-downloads

# 构建工具和库
sudo apt update
sudo apt install -y \
  build-essential cmake git \
  libglfw3-dev libglew-dev libglm-dev \
  libhdf5-dev

# 验证 CUDA
nvcc --version
```

## 构建

```bash
git clone https://github.com/AICL-Lab/n-body.git
cd n-body
./scripts/build.sh
```

## 验证安装

```bash
./build/nbody_sim 10000
./scripts/test.sh
```

## 下一步

- [快速开始](/zh-CN/getting-started/quick-start) - 运行你的第一个模拟
- [示例](/zh-CN/getting-started/examples) - 探索示例程序
