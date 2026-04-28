---
layout: docs-zh
title: 快速入门
lang: zh-CN
description: N-Body 粒子仿真系统的完整安装、构建和运行指南
---

# 快速入门指南

N-Body 粒子仿真系统的完整安装、构建和运行指南。

---

## 📋 目录

1. [系统要求](#系统要求)
2. [安装](#安装)
3. [构建](#构建项目)
4. [运行](#运行仿真)
5. [下一步](#下一步)
6. [故障排除](#故障排除)

---

## 系统要求

### 硬件要求

| 组件 | 最低配置 | 推荐配置 | 说明 |
|------|----------|----------|------|
| **GPU** | NVIDIA GTX 1060 | RTX 3080+ | 需要计算能力 7.0+ |
| **显存** | 2 GB | 8 GB+ | 100万+粒子所需 |
| **内存** | 8 GB | 16 GB+ | 主机数据传输 |
| **存储** | 500 MB | 1 GB | 构建产物和依赖 |

### 软件要求

| 组件 | 版本 | 安装命令 |
|------|------|----------|
| **CUDA Toolkit** | 11.0+ | [NVIDIA CUDA](https://developer.nvidia.com/cuda-downloads) |
| **CMake** | 3.18+ | `sudo apt install cmake` |
| **GCC/Clang** | C++17 | 通常已包含 |
| **OpenGL** | 3.3+ | 通常已包含 |
| **GLFW** | 3.3+ | `sudo apt install libglfw3-dev` |
| **GLEW** | 2.1+ | `sudo apt install libglew-dev` |
| **GLM** | 0.9.9+ | `sudo apt install libglm-dev` |

### 验证 CUDA 安装

```bash
# 检查 CUDA 编译器
nvcc --version

# 检查 GPU 检测
nvidia-smi
```

预期输出应显示 CUDA 版本和 GPU 详情。

---

## 安装

### Linux (Ubuntu/Debian)

```bash
# 1. 安装依赖
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libglfw3-dev \
    libglew-dev \
    libglm-dev

# 2. 克隆仓库
git clone https://github.com/LessUp/n-body.git
cd n-body

# 3. 验证目录结构
ls -la
# 应显示: CMakeLists.txt, src/, include/, tests/, docs/
```

### Windows (Visual Studio)

1. 安装 **Visual Studio 2019+** 并选择 C++ 工作负载
2. 从 [NVIDIA](https://developer.nvidia.com/cuda-downloads) 安装 **CUDA Toolkit**
3. 安装 **CMake 3.18+**
4. 通过 **vcpkg** 安装依赖：

```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install glfw3 glew glm
```

---

## 构建项目

### Linux 构建

```bash
# 创建构建目录
mkdir -p build && cd build

# 配置（Release 模式以获得最佳性能）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 使用所有 CPU 核心构建
cmake --build . -j$(nproc)
```

### Windows 构建

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### 构建选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` 或 `Release` |
| `NBODY_BUILD_TESTS` | `ON` | 构建测试套件 |
| `CMAKE_CUDA_ARCHITECTURES` | `native` | GPU 架构（如 RTX 30xx 用 `86`） |

自定义选项示例：

```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_ARCHITECTURES=86 \
    -DNBODY_BUILD_TESTS=ON
```

---

## 运行仿真

### 基本用法

```bash
# 默认：10,000 粒子
./nbody_sim

# 自定义粒子数量
./nbody_sim 100000    # 10万粒子
./nbody_sim 1000000   # 100万粒子
./nbody_sim 5000000   # 500万粒子（需要 8GB+ 显存）
```

### 交互控制

| 按键 | 功能 |
|------|------|
| `Space` | ⏯️ 暂停/继续 |
| `R` | 🔄 重置仿真 |
| `1` | 🔢 Direct N² 算法 |
| `2` | 🌳 Barnes-Hut 算法 |
| `3` | 🔲 Spatial Hash 算法 |
| `C` | 📷 重置相机 |
| `Esc` | ❌ 退出 |
| **鼠标** | |
| `左键拖动` | 🔄 旋转视角 |
| `滚轮` | 🔍 缩放 |

### 算法选择指南

| 粒子数 | 推荐算法 | 原因 |
|--------|----------|------|
| < 1万 | Direct N²（按 `1`） | 最快、最精确 |
| 1-10万 | Barnes-Hut（按 `2`） | 速度与精度平衡 |
| > 10万 | Spatial Hash（按 `3`） | 最佳性能 |

---

## 故障排除

### 常见问题

#### 1. 找不到 CUDA

**错误：**
```
CMake Error: Could not find CUDA
```

**解决方案：**
```bash
# 验证 CUDA 安装
nvcc --version

# 设置 CUDA 路径（如需要）
export CUDA_HOME=/usr/local/cuda
export PATH=$CUDA_HOME/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH

# 重新配置 CMake
cmake .. -DCMAKE_CUDA_COMPILER=$CUDA_HOME/bin/nvcc
```

#### 2. 显存不足

**错误：**
```
CUDA Error: out of memory
```

**解决方案：**
- 减少粒子数：`./nbody_sim 50000`
- 检查显存：`nvidia-smi`
- 关闭其他 GPU 应用

---

## 性能速查

RTX 3080 测试数据：

| 粒子数 | Direct N² | Barnes-Hut | Spatial Hash |
|--------|-----------|------------|--------------|
| 1万 | 60+ FPS | 120+ FPS | 120+ FPS |
| 10万 | ~10 FPS | 60+ FPS | 90+ FPS |
| 100万 | N/A | ~25 FPS | 60+ FPS |
