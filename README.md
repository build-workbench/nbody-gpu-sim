# N-Body Particle Simulation System

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/CUDA-11.0+-76B900?logo=nvidia&logoColor=white" alt="CUDA">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white" alt="C++">
  <img src="https://img.shields.io/badge/OpenGL-3.3+-5586A4?logo=opengl&logoColor=white" alt="OpenGL">
  <img src="https://img.shields.io/badge/CMake-3.18+-064F8C?logo=cmake&logoColor=white" alt="CMake">
</p>

<p align="center">
  <b>High-Performance GPU-Accelerated N-Body Simulation with Real-Time Visualization</b>
</p>

<p align="center">
  <a href="https://github.com/LessUp/n-body/releases"><img src="https://img.shields.io/github/v/release/LessUp/n-body?include_prereleases" alt="Latest Release"></a>
  <a href="https://github.com/LessUp/n-body/issues"><img src="https://img.shields.io/github/issues/LessUp/n-body" alt="Issues"></a>
  <img src="https://img.shields.io/github/stars/LessUp/n-body?style=social" alt="Stars">
</p>

<p align="center">
  English | <a href="README.zh-CN.md">简体中文</a>
</p>

---

## ✨ Features

- 🚀 **High-Performance GPU Computing** — CUDA parallel computation, million-particle real-time simulation
- 🔬 **Multiple Force Algorithms** — Direct N², Barnes-Hut O(N log N), Spatial Hash O(N)
- 🎨 **Real-Time Visualization** — CUDA-OpenGL interop, no CPU-GPU data transfer
- ⚛️ **Energy Conservation** — Velocity Verlet symplectic integrator
- 🎮 **Interactive Controls** — Camera rotation/zoom, runtime algorithm switching
- 🧪 **Comprehensive Testing** — Google Test + RapidCheck property-based testing
- 📦 **Easy to Build** — CMake-based, cross-platform

---

## 🚀 Quick Start

### Prerequisites

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| GPU | NVIDIA CC 7.0+ | NVIDIA CC 8.0+ |
| CUDA | 11.0 | 12.0+ |
| CMake | 3.18 | 3.25+ |
| OpenGL | 3.3 | 4.5+ |

### Installation (Ubuntu)

```bash
# Install dependencies
sudo apt-get install -y cmake libglfw3-dev libglew-dev libglm-dev

# Clone repository
git clone https://github.com/LessUp/n-body.git
cd n-body

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# Run
./nbody_sim 100000    # 100K particles
./nbody_sim 1000000   # 1 million particles!
```

### Windows

See [Getting Started Guide](docs/en/getting-started.md) for Windows build instructions.

---

## 📊 Performance

Tested on NVIDIA RTX 3080:

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|-----------|-----------|------------|--------------|
| 10K | 60 FPS | 120+ FPS | 120+ FPS |
| 100K | ~10 FPS | 60+ FPS | 90+ FPS |
| 1M | — | 25+ FPS | 60+ FPS |

**Memory Usage**: ~52 bytes per particle + algorithm overhead

---

## 🎮 Controls

| Key | Action |
|-----|--------|
| `Space` | ⏯️ Pause/Resume |
| `R` | 🔄 Reset simulation |
| `1/2/3` | 🔀 Switch algorithm |
| `C` | 📷 Reset camera |
| `Esc` | ❌ Exit |
| `Mouse Drag` | 🔄 Rotate view |
| `Scroll` | 🔍 Zoom |

---

## 📚 Documentation

### Getting Started

- [Getting Started Guide](docs/setup/getting-started.md) — Complete setup and usage
- [Examples](examples/) — Code examples for common use cases

### Reference

- [Architecture](docs/architecture/architecture.md) — System design and components
- [Algorithms](docs/architecture/algorithms.md) — Algorithm explanations
- [API Reference](docs/architecture/api.md) — Complete API documentation
- [Performance Guide](docs/architecture/performance.md) — Optimization strategies

### Specifications (Single Source of Truth)

- [Product Spec](specs/product/n-body-simulation.md) — Requirements and acceptance criteria
- [Architecture RFC](specs/rfc/0001-core-architecture.md) — Technical design decisions

### 🌐 Available Languages

| Language | Documentation |
|----------|---------------|
| 🇺🇸 English | [Docs](./docs/) |
| 🇨🇳 简体中文 | [文档](./docs/zh-CN/) |

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────┐
│  Application (GLFW, Input, State)       │
├─────────────────────────────────────────┤
│  Simulation (ParticleSystem, Forces)    │
├─────────────────────────────────────────┤
│  Rendering (OpenGL, Camera)             │
├─────────────────────────────────────────┤
│  GPU Memory (CUDA, Shared VBO)          │
└─────────────────────────────────────────┘
```

See [Architecture Documentation](docs/en/architecture.md) for details.

---

## 🧪 Testing

```bash
cd build
./nbody_tests

# Run specific suite
./nbody_tests --gtest_filter=ForceCalculation.*
```

---

## 💡 Usage Example

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

## 🔬 Algorithms

| Algorithm | Complexity | Best For |
|-----------|------------|----------|
| Direct N² | O(N²) | Small systems, precision testing |
| Barnes-Hut | O(N log N) | Large-scale gravity |
| Spatial Hash | O(N) | Short-range forces |

See [Algorithms Documentation](docs/en/algorithms.md) for details.

---

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

## 📄 License

MIT License — See [LICENSE](LICENSE) for details.

---

## 🌟 Star History

[![Star History Chart](https://api.star-history.com/svg?repos=LessUp/n-body&type=Date)](https://star-history.com/#LessUp/n-body&Date)

---

## 📖 Citation

If you use this project in your research, please cite:

```bibtex
@software{nbody_simulation,
  title = {N-Body Particle Simulation System},
  author = {N-Body Simulation Team},
  url = {https://github.com/LessUp/n-body},
  year = {2025}
}
```

---

## Related Projects

- [Barnes & Hut (1986)](https://doi.org/10.1038/324446a0) — Original Barnes-Hut algorithm
- [GPU Gems 3](https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-31-fast-n-body-simulation-cuda) — CUDA N-Body simulation
- [NVIDIA CUDA Samples](https://github.com/NVIDIA/cuda-samples) — Official CUDA samples
