# Installation

This guide covers installing and building the N-Body simulation on your system.

## Requirements

### Hardware

- **GPU**: NVIDIA GPU with CUDA compute capability 3.0+
- **RAM**: 8 GB minimum, 16 GB recommended for 1M particles
- **Storage**: ~500 MB for build artifacts

### Software

| Dependency | Version | Notes |
|------------|---------|-------|
| CUDA Toolkit | 11.0+ | Required for GPU acceleration |
| CMake | 3.18+ | Build system |
| C++ Compiler | C++20 | GCC 10+, Clang 12+, MSVC 19.28+ |
| OpenGL | 3.3+ | For visualization |
| GLFW | 3.3+ | Window management |
| GLEW | 2.1+ | OpenGL extensions |
| GLM | 0.9.9+ | Math library |

### Optional

| Dependency | Version | Purpose |
|------------|---------|---------|
| HDF5 | 1.14+ | Scientific data export |
| Python | 3.8+ | Analysis scripts |

## Install Dependencies

### Ubuntu/Debian

```bash
# CUDA (follow NVIDIA's guide for your GPU)
# https://developer.nvidia.com/cuda-downloads

# Build tools and libraries
sudo apt update
sudo apt install -y \
  build-essential cmake git \
  libglfw3-dev libglew-dev libglm-dev \
  libhdf5-dev

# Verify CUDA
nvcc --version
```

### Fedora/RHEL

```bash
# CUDA (follow NVIDIA's guide)
sudo dnf install -y \
  gcc-c++ cmake git \
  glfw-devel glew-devel glm-devel \
  hdf5-devel
```

### macOS

```bash
# Homebrew
brew install cmake glfw glew glm hdf5

# CUDA on macOS requires external GPU or cloud
```

### Windows

1. Install [Visual Studio 2019+](https://visualstudio.microsoft.com/) with C++ workload
2. Install [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)
3. Install [CMake](https://cmake.org/download/)
4. Use vcpkg for dependencies:

```powershell
vcpkg install glfw3 glew glm hdf5
```

## Build

### Quick Build (Linux/macOS)

```bash
git clone https://github.com/AICL-Lab/n-body.git
cd n-body
./scripts/build.sh
```

The script auto-detects CUDA and falls back to headless mode if unavailable.

### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `NBODY_ENABLE_CUDA` | ON | CUDA support |
| `NBODY_ENABLE_RENDERING` | ON | OpenGL visualization |
| `NBODY_ENABLE_UI` | ON | Dear ImGui panel |
| `NBODY_ENABLE_HDF5` | ON | HDF5 export |
| `NBODY_BUILD_TESTS` | ON | Unit tests |
| `NBODY_BUILD_EXAMPLES` | ON | Example programs |
| `NBODY_BUILD_BENCHMARKS` | ON | Benchmarks |

Example:

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DNBODY_ENABLE_PROFILING=ON \
  -DNBODY_BUILD_TESTS=ON
```

### Headless Build (No GPU)

For CI or systems without NVIDIA GPU:

```bash
mkdir build/headless && cd build/headless
cmake ../.. \
  -DCMAKE_BUILD_TYPE=Release \
  -DNBODY_ENABLE_RENDERING=OFF \
  -DNBODY_ENABLE_CUDA=OFF \
  -DNBODY_BUILD_TESTS=ON \
  -DNBODY_BUILD_BENCHMARKS=ON \
  -DNBODY_BUILD_EXAMPLES=OFF
cmake --build . -j$(nproc)
```

## Verify Installation

```bash
# Run the simulation
./build/nbody_sim 10000

# Run tests
./scripts/test.sh

# Run benchmarks
./scripts/benchmark.sh
```

## Troubleshooting

### CUDA Not Found

```bash
# Set CUDA path
export CUDA_HOME=/usr/local/cuda
export PATH=$CUDA_HOME/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
```

### OpenGL Errors

```bash
# Check OpenGL version
glxinfo | grep "OpenGL version"

# Ensure display is available
echo $DISPLAY
```

### Out of Memory

Reduce particle count or use a more efficient algorithm:

```bash
# Use Barnes-Hut for large systems
./build/nbody_sim 1000000 --algorithm barnes-hut
```

## Next Steps

- [Quick Start](/en/getting-started/quick-start) - Run your first simulation
- [Examples](/en/getting-started/examples) - Explore example programs
- [Configuration](/en/user-guide/configuration) - Customize simulation parameters
