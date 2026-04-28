---
layout: default
title: 快速入门
parent: 文档
nav_order: 1
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

如果只需要验证 **headless core-only** 构建路径，可以跳过 CUDA / OpenGL 依赖，并在配置阶段关闭 rendering 和 examples，同时保留 headless 可观测性测试与 benchmark。

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

### Headless Core-Only 构建

当机器上没有 CUDA 或 OpenGL 开发包，但仍需要验证仓库的非可视化核心构建路径时，可以使用下面的配置：

```bash
mkdir -p build/headless && cd build/headless

cmake ../.. \
    -DCMAKE_BUILD_TYPE=Release \
    -DNBODY_ENABLE_RENDERING=OFF \
    -DNBODY_ENABLE_CUDA=OFF \
    -DNBODY_BUILD_TESTS=ON \
    -DNBODY_BUILD_BENCHMARKS=ON \
    -DNBODY_BUILD_EXAMPLES=OFF

cmake --build . -j$(nproc)
```

该配置当前会生成核心静态库 `libnbody_lib.a`、`nbody_observability_tests` 测试目标以及 `nbody_benchmarks` 可执行程序，并有意跳过渲染程序、示例和依赖 CUDA 的完整仿真测试。

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
| `NBODY_ENABLE_RENDERING` | `ON` | 是否构建 OpenGL/GLFW 可视化面 |
| `NBODY_BUILD_TESTS` | `ON` | 构建测试套件 |
| `NBODY_BUILD_BENCHMARKS` | `ON` | 构建 `nbody_benchmarks` 可执行程序 |
| `NBODY_ENABLE_PROFILING` | `OFF` | 在 benchmark 输出中启用命名阶段 timing |
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

### 界面说明

窗口标题格式：
```
N-Body Simulation | 100000 particles | 60.0 FPS | Time: 12.34
```

- **Particles**: 当前粒子数
- **FPS**: 每秒帧数（目标：60+）
- **Time**: 仿真经过时间

### 算法选择指南

| 粒子数 | 推荐算法 | 原因 |
|--------|----------|------|
| < 1万 | Direct N²（按 `1`） | 最快、最精确 |
| 1-10万 | Barnes-Hut（按 `2`） | 速度与精度平衡 |
| > 10万 | Spatial Hash（按 `3`） | 最佳性能 |

---

## 运行测试

```bash
./scripts/test.sh
```

`./scripts/test.sh` 通过 `ctest` 运行已发现的测试，因此 headless 构建会执行 observability 测试，而 CUDA 构建还会覆盖完整仿真测试集。

## 📊 运行 Benchmark

```bash
./scripts/benchmark.sh
./scripts/benchmark.sh serialization.round_trip build/benchmark-results.json
```

Benchmark 可执行程序会输出结构化 JSON。若在配置时启用 `-DNBODY_ENABLE_PROFILING=ON`，结果中还会包含已接入阶段的命名 timing 数据。

测试套件：
- `ForceCalculation.*` - 力计算正确性
- `BarnesHut.*` - 树构建和遍历
- `SpatialHash.*` - 网格操作
- `Integrator.*` - 时间积分
- `Serialization.*` - 保存/加载功能

---

## 下一步

### 探索代码

| 示例 | 文件 | 说明 |
|------|------|------|
| 基础 | `examples/example_basic.cpp` | 最小仿真设置 |
| 算法 | `examples/example_force_methods.cpp` | 算法对比 |
| 分布 | `examples/example_custom_distribution.cpp` | 自定义初始条件 |
| 能量 | `examples/example_energy_conservation.cpp` | 能量监控 |

### 阅读文档

1. [架构](./architecture.md) - 了解系统设计
2. [算法](./algorithms.md) - 学习力计算方法
3. [API 参考](./api.md) - 程序化使用库
4. [性能指南](./performance.md) - 针对用例优化

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

#### 2. 找不到 GLFW/GLEW

**错误：**
```
Could not find glfw3
```

**解决方案：**
```bash
# Ubuntu/Debian
sudo apt-get install libglfw3-dev libglew-dev

# Fedora
sudo dnf install glfw-devel glew-devel

# macOS
brew install glfw glew
```

#### 3. 显存不足

**错误：**
```
CUDA Error: out of memory
```

**解决方案：**
- 减少粒子数：`./nbody_sim 50000`
- 检查显存：`nvidia-smi`
- 关闭其他 GPU 应用
- 降低算法精度（增大 Barnes-Hut 的 θ）

#### 4. 帧率低

**原因与解决方案：**

| 症状 | 原因 | 解决方案 |
|------|------|----------|
| <10万粒子 FPS < 10 | Debug 构建 | 使用 `-DCMAKE_BUILD_TYPE=Release` 重建 |
| >10万粒子 FPS < 10 | 算法选择错误 | 按 `2` 或 `3` 切换算法 |
| FPS 随时间下降 | 内存泄漏 | 更新到最新版本 |
| FPS 不稳定 | 驱动问题 | 更新 NVIDIA 驱动 |

#### 5. 构建失败

**清理重建：**
```bash
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

**详细输出：**
```bash
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
cmake --build . 2>&1 | tee build.log
```

---

## 性能速查

RTX 3080 测试数据：

| 粒子数 | Direct N² | Barnes-Hut | Spatial Hash |
|--------|-----------|------------|--------------|
| 1万 | 60+ FPS | 120+ FPS | 120+ FPS |
| 10万 | ~10 FPS | 60+ FPS | 90+ FPS |
| 100万 | N/A | ~25 FPS | 60+ FPS |

---

## 获取帮助

1. 首先查看本指南
2. 查阅 [GitHub Issues](https://github.com/LessUp/n-body/issues)
3. 创建新问题，包含：
   - GPU 型号和驱动版本
   - CUDA 版本（`nvcc --version`）
   - 操作系统
   - 完整错误信息
   - 复现步骤
