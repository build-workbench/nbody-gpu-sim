# 快速上手

构建、运行 N-Body 粒子模拟的完整指南。

## 环境要求

### 硬件

| 组件 | 最低 | 推荐 | 说明 |
|------|------|------|------|
| GPU | NVIDIA GTX 1650 | RTX 3080+ | 需 Compute Capability 7.5+（Turing 及以上） |
| 显存 | 2 GB | 8 GB+ | 100 万粒子所需 |
| 内存 | 8 GB | 16 GB+ | 主机端数据传输 |
| 存储 | 500 MB | 1 GB | 构建产物与依赖 |

### 软件

| 组件 | 版本 | 安装 |
|------|------|------|
| CUDA Toolkit | 11.8+ | [NVIDIA CUDA](https://developer.nvidia.com/cuda-downloads) |
| CMake | 3.18+ | `sudo apt install cmake` |
| GCC/Clang | C++17 支持 | 通常已自带 |
| OpenGL | 3.3+ | 通常已自带 |
| GLFW | 3.3+ | `sudo apt install libglfw3-dev` |
| GLEW | 2.1+ | `sudo apt install libglew-dev` |
| GLM | 0.9.9+ | `sudo apt install libglm-dev` |

无 CUDA/OpenGL 开发包时，可走**无头核心构建**路径，仍能跑核心测试与基准。

### 验证 CUDA 安装

```bash
nvcc --version
nvidia-smi
```

## 安装

### Linux（Ubuntu/Debian）

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libglfw3-dev libglew-dev libglm-dev
git clone https://github.com/build-workbench/nbody-gpu-sim.git
cd nbody-gpu-sim
```

### Windows（Visual Studio）

1. 安装 **Visual Studio 2019+** 及 C++ 工作负载
2. 安装 **CUDA Toolkit**
3. 安装 **CMake 3.18+**
4. 通过 **vcpkg** 安装依赖：

```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install glfw3 glew glm
```

## 构建

### 标准 Linux 构建

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

或直接用脚本：`./scripts/build.sh`

### 无头核心构建（无 CUDA/OpenGL）

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

产出核心静态库 `libnbody_lib.a`、`nbody_core_tests`（CLI、可观测性、序列化、校验测试）和 `nbody_benchmarks`。跳过渲染可执行文件、示例和 CUDA 模拟测试。

### Windows

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### 构建选项

| 选项 | 默认 | 说明 |
|------|------|------|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` 或 `Release` |
| `NBODY_ENABLE_RENDERING` | `ON` | 构建 OpenGL/GLFW 可视化 |
| `NBODY_BUILD_TESTS` | `ON` | 构建测试套件 |
| `NBODY_BUILD_BENCHMARKS` | `ON` | 构建 `nbody_benchmarks` |
| `NBODY_ENABLE_PROFILING` | `OFF` | 基准输出中启用命名阶段计时 |
| `CMAKE_CUDA_ARCHITECTURES` | `native`（CMake 3.24+） | GPU 架构（如 `86` 对应 RTX 30xx）。旧版 CMake 回退到 `75;80;86;89;90` |

## 运行

```bash
./nbody_sim              # 默认 1 万粒子
./nbody_sim 100000       # 10 万
./nbody_sim 1000000      # 100 万
./nbody_sim 5000000      # 500 万（需 8GB+ 显存）
```

### 交互控制

| 按键 | 动作 |
|------|------|
| `Space` | 暂停/恢复 |
| `R` | 重置模拟 |
| `1` / `2` / `3` | 切换 Direct N² / Barnes-Hut / Spatial Hash |
| `C` | 重置相机 |
| `Esc` | 退出 |
| 鼠标左键拖拽 | 旋转视角 |
| 滚轮 | 缩放 |

窗口标题实时显示粒子数、FPS 和模拟时间。

## 测试与基准

```bash
./scripts/test.sh
./scripts/benchmark.sh
./scripts/benchmark.sh serialization.round_trip build/benchmark-results.json
```

基准可执行文件输出结构化 JSON；配置 `-DNBODY_ENABLE_PROFILING=ON` 时额外包含命名阶段计时。

## 排错

### 找不到 CUDA

```bash
nvcc --version
export CUDA_HOME=/usr/local/cuda
export PATH=$CUDA_HOME/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
cmake .. -DCMAKE_CUDA_COMPILER=$CUDA_HOME/bin/nvcc
```

### 找不到 GLFW/GLEW

```bash
sudo apt-get install libglfw3-dev libglew-dev   # Ubuntu/Debian
sudo dnf install glfw-devel glew-devel           # Fedora
brew install glfw glew                            # macOS
```

### 显存不足

- 减少粒子数：`./nbody_sim 50000`
- 检查显存：`nvidia-smi`
- 关闭其他 GPU 应用

### FPS 偏低

| 症状 | 原因 | 解决 |
|------|------|------|
| <100K 粒子时 FPS<10 | Debug 构建 | 用 `-DCMAKE_BUILD_TYPE=Release` 重建 |
| >100K 粒子时 FPS<10 | 算法不当 | 按 `2` 或 `3` 切换算法 |
| FPS 随时间下降 | 驱动问题 | 更新 NVIDIA 驱动 |

### 构建失败

```bash
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

详细日志：`cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON && cmake --build . 2>&1 | tee build.log`

## 下一步

- [示例程序](../../examples/README.md) — 可运行的用法演示
- [项目 README](../../README.md) — 概览与快速开始
- [AGENTS.md](../../AGENTS.md) — 架构说明与 AI 协作指引

## 获取帮助

1. 先查阅本指南
2. 查看 [GitHub Issues](https://github.com/build-workbench/nbody-gpu-sim/issues)
3. 提交新 issue 时附上：GPU 型号与驱动版本、CUDA 版本（`nvcc --version`）、操作系统、完整错误信息、复现步骤
