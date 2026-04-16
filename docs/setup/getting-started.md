---
layout: default
title: Getting Started
parent: Documentation
nav_order: 1
---

# Getting Started Guide

Complete guide for setting up, building, and running the N-Body Particle Simulation System.

---

## 📋 Table of Contents

1. [Requirements](#-system-requirements)
2. [Installation](#-installation)
3. [Building](#-building-the-project)
4. [Running](#-running-the-simulation)
5. [Next Steps](#-next-steps)
6. [Troubleshooting](#-troubleshooting)

---

## 📋 System Requirements

### Hardware Requirements

| Component | Minimum | Recommended | Notes |
|-----------|---------|-------------|-------|
| **GPU** | NVIDIA GTX 1060 | RTX 3080+ | Compute Capability 7.0+ required |
| **VRAM** | 2 GB | 8 GB+ | For 1M+ particles |
| **RAM** | 8 GB | 16 GB+ | Host memory for data transfer |
| **Storage** | 500 MB | 1 GB | Build artifacts and dependencies |

### Software Requirements

| Component | Version | Installation |
|-----------|---------|--------------|
| **CUDA Toolkit** | 11.0+ | [NVIDIA CUDA](https://developer.nvidia.com/cuda-downloads) |
| **CMake** | 3.18+ | `sudo apt install cmake` |
| **GCC/Clang** | C++17 support | Usually included |
| **OpenGL** | 3.3+ | Usually included |
| **GLFW** | 3.3+ | `sudo apt install libglfw3-dev` |
| **GLEW** | 2.1+ | `sudo apt install libglew-dev` |
| **GLM** | 0.9.9+ | `sudo apt install libglm-dev` |

### Verify CUDA Installation

```bash
# Check CUDA compiler
nvcc --version

# Check GPU detection
nvidia-smi
```

Expected output should show CUDA version and GPU details.

---

## 💾 Installation

### Linux (Ubuntu/Debian)

```bash
# 1. Install dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libglfw3-dev \
    libglew-dev \
    libglm-dev

# 2. Clone the repository
git clone https://github.com/LessUp/n-body.git
cd n-body

# 3. Verify structure
ls -la
# Should show: CMakeLists.txt, src/, include/, tests/, docs/
```

### Windows (Visual Studio)

1. Install **Visual Studio 2019+** with C++ workload
2. Install **CUDA Toolkit** from [NVIDIA](https://developer.nvidia.com/cuda-downloads)
3. Install **CMake 3.18+**
4. Install dependencies via **vcpkg**:

```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install glfw3 glew glm
```

---

## 🔨 Building the Project

### Linux Build

```bash
# Create build directory
mkdir -p build && cd build

# Configure (Release mode for performance)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build using all CPU cores
cmake --build . -j$(nproc)
```

### Windows Build

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` or `Release` |
| `NBODY_BUILD_TESTS` | `ON` | Build test suite |
| `CMAKE_CUDA_ARCHITECTURES` | `native` | GPU architecture (e.g., `86` for RTX 30xx) |

Example with custom options:

```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CUDA_ARCHITECTURES=86 \
    -DNBODY_BUILD_TESTS=ON
```

---

## 🎮 Running the Simulation

### Basic Usage

```bash
# Default: 10,000 particles
./nbody_sim

# Custom particle count
./nbody_sim 100000    # 100K particles
./nbody_sim 1000000   # 1 million particles
./nbody_sim 5000000   # 5 million particles (needs 8GB+ VRAM)
```

### Interactive Controls

| Key | Action |
|-----|--------|
| `Space` | ⏯️ Pause/Resume |
| `R` | 🔄 Reset simulation |
| `1` | 🔢 Direct N² algorithm |
| `2` | 🌳 Barnes-Hut algorithm |
| `3` | 🔲 Spatial Hash algorithm |
| `C` | 📷 Reset camera |
| `Esc` | ❌ Exit |
| **Mouse** | |
| `Left Drag` | 🔄 Rotate view |
| `Scroll` | 🔍 Zoom in/out |

### Understanding the Display

Window title format:
```
N-Body Simulation | 100000 particles | 60.0 FPS | Time: 12.34
```

- **Particles**: Current particle count
- **FPS**: Frames per second (target: 60+)
- **Time**: Simulation elapsed time

### Algorithm Selection Guide

| Particles | Recommended | Why |
|-----------|-------------|-----|
| < 10K | Direct N² (press `1`) | Fastest, most accurate |
| 10K - 100K | Barnes-Hut (press `2`) | Good speed/accuracy balance |
| > 100K | Spatial Hash (press `3`) | Best performance |

---

## 🧪 Running Tests

```bash
cd build

# Run all tests
./nbody_tests

# Run specific test suite
./nbody_tests --gtest_filter=ForceCalculation.*
./nbody_tests --gtest_filter=BarnesHut.*
./nbody_tests --gtest_filter=Integrator.*
```

Test suites:
- `ForceCalculation.*` - Force computation correctness
- `BarnesHut.*` - Tree construction and traversal
- `SpatialHash.*` - Grid operations
- `Integrator.*` - Time integration
- `Serialization.*` - Save/load functionality

---

## 📚 Next Steps

### Explore the Code

| Example | File | Description |
|---------|------|-------------|
| Basic | `examples/example_basic.cpp` | Minimal simulation setup |
| Algorithms | `examples/example_force_methods.cpp` | Compare algorithms |
| Distribution | `examples/example_custom_distribution.cpp` | Custom initial conditions |
| Energy | `examples/example_energy_conservation.cpp` | Monitor energy |

### Read Documentation

1. [Architecture](./architecture.md) - Understand system design
2. [Algorithms](./algorithms.md) - Learn force calculation methods
3. [API Reference](./api.md) - Use the library programmatically
4. [Performance Guide](./performance.md) - Optimize for your use case

---

## 🔧 Troubleshooting

### Common Issues

#### 1. CUDA Not Found

**Error:**
```
CMake Error: Could not find CUDA
```

**Solution:**
```bash
# Verify CUDA installation
nvcc --version

# Set CUDA path (if needed)
export CUDA_HOME=/usr/local/cuda
export PATH=$CUDA_HOME/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH

# Reconfigure CMake
cmake .. -DCMAKE_CUDA_COMPILER=$CUDA_HOME/bin/nvcc
```

#### 2. GLFW/GLEW Not Found

**Error:**
```
Could not find glfw3
```

**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install libglfw3-dev libglew-dev

# Fedora
sudo dnf install glfw-devel glew-devel

# macOS
brew install glfw glew
```

#### 3. Out of Memory

**Error:**
```
CUDA Error: out of memory
```

**Solutions:**
- Reduce particle count: `./nbody_sim 50000`
- Check VRAM: `nvidia-smi`
- Close other GPU applications
- Lower algorithm precision (increase θ for Barnes-Hut)

#### 4. Low FPS

**Causes & Solutions:**

| Symptom | Cause | Solution |
|---------|-------|----------|
| FPS < 10 with <100K | Debug build | Rebuild with `-DCMAKE_BUILD_TYPE=Release` |
| FPS < 10 with >100K | Wrong algorithm | Press `2` or `3` to switch |
| FPS drops over time | Memory leak | Update to latest version |
| Inconsistent FPS | Driver issue | Update NVIDIA drivers |

#### 5. Build Failures

**Clean build:**
```bash
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

**Verbose output:**
```bash
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
cmake --build . 2>&1 | tee build.log
```

---

## 📊 Performance Quick Reference

Tested on RTX 3080:

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|-----------|-----------|------------|--------------|
| 10,000 | 60+ FPS | 120+ FPS | 120+ FPS |
| 100,000 | ~10 FPS | 60+ FPS | 90+ FPS |
| 1,000,000 | N/A | ~25 FPS | 60+ FPS |

---

## 🆘 Getting Help

1. Check this guide first
2. Review [GitHub Issues](https://github.com/LessUp/n-body/issues)
3. Create new issue with:
   - GPU model and driver version
   - CUDA version (`nvcc --version`)
   - Operating system
   - Full error message
   - Steps to reproduce
