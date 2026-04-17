# N-Body Particle Simulation

<p align="center">
  <b>High-Performance GPU Physics Engine with Real-Time Visualization</b>
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License"></a>
  <a href="https://github.com/LessUp/n-body/releases"><img src="https://img.shields.io/github/v/release/LessUp/n-body?include_prereleases&label=Release" alt="Release"></a>
  <img src="https://img.shields.io/badge/CUDA-11.0+-76B900?logo=nvidia&logoColor=white" alt="CUDA">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white" alt="C++">
  <img src="https://img.shields.io/badge/OpenGL-3.3+-5586A4?logo=opengl" alt="OpenGL">
</p>

<p align="center">
  Simulate up to <b>1 million particles</b> in real-time using three force algorithms entirely on GPU
</p>

<p align="center">
  <a href="#-quick-start">Quick Start</a> •
  <a href="#-documentation">Documentation</a> •
  <a href="#-algorithms">Algorithms</a> •
  <a href="#-performance">Performance</a> •
  <a href="#-contributing">Contributing</a>
</p>

<p align="center">
  🇺🇸 <a href="README.md">English</a> | 🇨🇳 <a href="README.zh-CN.md">简体中文</a>
</p>

---

## ✨ Features

<div align="center">

| 🚀 Performance | 🔬 Algorithms | 🎨 Visualization |
|:---:|:---:|:---:|
| Million-particle simulation | Direct N² (O(N²)) | Zero-copy CUDA-OpenGL rendering |
| Real-time 60+ FPS | Barnes-Hut (O(N log N)) | Interactive camera controls |
| Energy-conserving integration | Spatial Hash (O(N)) | Customizable particle sizes |

</div>

**Key Highlights:**
- ⚡ **GPU-Accelerated** — All physics computation on CUDA with parallel execution
- 🔄 **Hot-Swap Algorithms** — Switch between force methods during simulation
- 🎯 **Symplectic Integrator** — Velocity Verlet ensures long-term energy conservation
- 🧪 **Production-Ready** — Comprehensive tests with Google Test + RapidCheck
- 📦 **Easy to Build** — CMake-based build system with convenience scripts

---

## 🚀 Quick Start

### Prerequisites

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **GPU** | NVIDIA CC 7.0+ | NVIDIA CC 8.0+ (RTX 3000+) |
| **CUDA** | 11.0 | 12.0+ |
| **CMake** | 3.18 | 3.25+ |
| **OpenGL** | 3.3 | 4.5+ |

### Build on Linux

```bash
# 1. Install dependencies
sudo apt-get install -y cmake libglfw3-dev libglew-dev libglm-dev

# 2. Clone and build
git clone https://github.com/LessUp/n-body.git
cd n-body
./scripts/build.sh

# 3. Run simulation
./build/nbody_sim 100000    # 100K particles
./build/nbody_sim 1000000   # 1 million particles!
```

### Build on Windows

See [Getting Started Guide](docs/setup/getting-started.md) for Windows instructions.

### Convenience Scripts

| Script | Description |
|--------|-------------|
| `./scripts/build.sh` | Build the project |
| `./scripts/test.sh` | Run test suite |
| `./scripts/format.sh` | Format code with clang-format |

---

## 🎮 Interactive Controls

| Input | Action |
|-------|--------|
| `Space` | ⏯️ Pause / Resume simulation |
| `1` / `2` / `3` | 🔀 Switch: Direct N² → Barnes-Hut → Spatial Hash |
| `R` | 🔄 Reset simulation |
| `C` | 📷 Reset camera view |
| `Esc` | ❌ Exit application |
| 🖱️ Drag | 🔄 Rotate 3D view |
| 🔲 Scroll | 🔍 Zoom in/out |

---

## 📊 Performance Benchmarks

Tested on **NVIDIA RTX 3080**, CUDA 11.8:

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|:---------:|:---------:|:----------:|:------------:|
| 10K | 60 FPS | 120+ FPS | 120+ FPS |
| 100K | ~10 FPS | 60+ FPS | 90+ FPS |
| **1M** | <1 FPS | 25+ FPS | **60+ FPS** |

> **Memory Footprint**: ~52 bytes per particle + algorithm-specific overhead  
> **Best for Large Scale**: Barnes-Hut for gravitational, Spatial Hash for short-range forces

---

## 🔬 Force Algorithms

| Algorithm | Time Complexity | Space Complexity | Best Use Case |
|-----------|----------------|-----------------|---------------|
| **Direct N²** | O(N²) | O(1) | Small systems, precision validation |
| **Barnes-Hut** | O(N log N) | O(N) | Large-scale gravitational simulation |
| **Spatial Hash** | O(N) | O(N) | Short-range forces, molecular dynamics |

**Algorithm Selection Guide:**
- Use **Direct N²** for ≤10K particles or accuracy validation
- Use **Barnes-Hut** for large-scale gravitational systems (adjust θ for accuracy/speed trade-off)
- Use **Spatial Hash** for short-range potentials with cutoff radius

📖 [Learn more about algorithms →](docs/architecture/algorithms.md)

---

## 🏗️ Architecture

```
┌──────────────────────────────────────────────────────┐
│                  CPU (Host)                          │
│  SimulationConfig ──▶ ParticleSystem ──▶ Render Loop │
└────────────────────────┬─────────────────────────────┘
                         │ CUDA-OpenGL Interop (zero-copy)
┌────────────────────────▼─────────────────────────────┐
│                  GPU (Device)                        │
│  ┌──────────────┐  ┌──────────────┐  ┌────────────┐ │
│  │ Force Kernel │  │ Velocity     │  │ OpenGL     │ │
│  │ N²/BH/Hash   │─▶│ Verlet Int.  │─▶│ Rendering  │ │
│  └──────────────┘  └──────────────┘  └────────────┘ │
└──────────────────────────────────────────────────────┘
```

**Design Patterns:** Strategy (force calculators), Bridge (CUDA-OpenGL), Facade (ParticleSystem API)  
**Data Layout:** Structure of Arrays (SoA) for GPU memory coalescing

📖 [Explore architecture details →](docs/architecture/architecture.md)

---

## 💡 Code Example

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    // Configure simulation
    SimulationConfig config;
    config.particle_count = 100'000;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;
    config.distribution = InitDistribution::UNIFORM_SPHERE;

    // Initialize and run
    ParticleSystem system;
    system.initialize(config);

    for (int step = 0; step < 10'000; ++step) {
        system.update(system.getTimeStep());
        
        // Optionally save checkpoints
        if (step % 1000 == 0) {
            system.saveState("checkpoint_" + std::to_string(step) + ".nbody");
        }
    }

    return 0;
}
```

---

## 📚 Documentation

### Getting Started
- **[Setup Guide](docs/setup/getting-started.md)** — Installation and first run
- **[Examples](examples/)** — Code examples for common scenarios

### Technical Reference
- **[Architecture](docs/architecture/architecture.md)** — System design and components
- **[Algorithms](docs/architecture/algorithms.md)** — Force calculation methods
- **[API Reference](docs/architecture/api.md)** — Complete API documentation
- **[Performance Guide](docs/architecture/performance.md)** — Optimization strategies

### Specifications (Single Source of Truth)
- **[Product Spec](specs/product/n-body-simulation.md)** — Requirements and acceptance criteria
- **[Architecture RFC](specs/rfc/0001-core-architecture.md)** — Technical design decisions

---

## 🧪 Testing

```bash
# Run all tests
./scripts/test.sh

# Or manually
cd build && ./nbody_tests

# Run specific test suite
./nbody_tests --gtest_filter=ForceCalculation.*

# Verbose output
./nbody_tests --gtest_color=yes --gtest_brief=0
```

---

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Code style guidelines
- Pull request process
- Development workflow
- Spec-driven development principles

**Quick Start for Contributors:**
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make changes and add tests
4. Run tests: `./scripts/test.sh`
5. Commit using conventional commits: `git commit -m "feat: add my feature"`
6. Push and open a Pull Request

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 🌟 Star History

[![Star History Chart](https://api.star-history.com/svg?repos=LessUp/n-body&type=Date)](https://star-history.com/#LessUp/n-body&Date)

---

## 📖 Citation

If you use this project in academic research, please cite it:

```bibtex
@software{nbody_simulation_2025,
  title   = {N-Body Particle Simulation System},
  author  = {N-Body Simulation Team},
  url     = {https://github.com/LessUp/n-body},
  year    = {2025},
  license = {MIT}
}
```

---

## 🔗 Related Resources

- **[Barnes & Hut (1986)](https://doi.org/10.1038/324446a0)** — Original O(N log N) algorithm
- **[GPU Gems 3, Ch. 31](https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-31-fast-n-body-simulation-cuda)** — Fast N-Body with CUDA
- **[NVIDIA CUDA Samples](https://github.com/NVIDIA/cuda-samples)** — Official CUDA code samples
- **[Velocity Verlet Integration](https://en.wikipedia.org/wiki/Verlet_integration)** — Symplectic integration method

---

<p align="center">
  <b>Ready to simulate the universe?</b> <a href="docs/setup/getting-started.md">Get Started →</a>
</p>
