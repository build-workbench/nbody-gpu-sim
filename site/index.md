---
layout: home
title: Home
nav_order: 1
description: "N-Body Simulation - Million-Particle GPU Physics Engine with Real-Time Visualization"
permalink: /
---

<div markdown="1" class="hero-section">

# N-Body Particle Simulation {: .fs-9 }

High-performance GPU physics engine with real-time visualization{: .fs-6 .text-green-300 }

</div>

<div markdown="1" class="grid grid-3-col">

<div markdown="1" class="card">

### 🚀 Million Particles

Simulate up to **1 million particles** in real-time on modern GPUs with CUDA parallel processing.

</div>

<div markdown="1" class="card">

### ⚡ Three Algorithms

Choose from **Direct N²**, **Barnes-Hut**, or **Spatial Hash** - hot-swap algorithms during simulation.

</div>

<div markdown="1" class="card">

### 🎨 Zero-Copy Rendering

CUDA-OpenGL interop eliminates CPU↔GPU data transfer for maximum performance.

</div>

</div>

---

## Performance Benchmarks

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|:---------:|:---------:|:----------:|:------------:|
| 10 K      | 60+ FPS   | 60+ FPS    | 60+ FPS      |
| 100 K     | ~10 FPS   | 60+ FPS    | 60+ FPS      |
| **1 M**   | <1 FPS    | ~30 FPS    | **60+ FPS**  |

*Tested on NVIDIA RTX 3080, CUDA 11.8*

## Key Features

- **Symplectic Integration** — Velocity Verlet ensures long-term energy conservation
- **Interactive Controls** — Camera orbit, zoom, and algorithm switching at runtime
- **Multiple Distributions** — Uniform sphere, shell, cube, or Gaussian initial conditions
- **Real-Time Visualization** — Point sprite rendering with customizable particle sizes
- **Cross-Platform** — Linux, Windows, macOS with CUDA-capable NVIDIA GPU

## Quick Start

```bash
# Install dependencies (Ubuntu)
sudo apt-get install -y cmake libglfw3-dev libglew-dev libglm-dev

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# Run simulation (default: 10K particles)
./nbody_sim

# Million-particle simulation
./nbody_sim 1000000
```

## Simulation Controls

| Key | Action |
|:---:|:-------|
| `1` / `2` / `3` | Switch Direct / Barnes-Hut / Spatial Hash |
| `Space` | Pause / resume |
| `R` | Reset simulation |
| `C` | Reset camera |
| `Esc` | Quit |
| 🖱️ Drag | Rotate view |
| 🔲 Scroll | Zoom |

## Architecture Overview

```
┌──────────────────────────────────────────────────────────────┐
│                     CPU  (Host)                              │
│  SimulationConfig ──▶ ParticleSystem ──▶ Render Loop         │
└──────────────────────────┬───────────────────────────────────┘
                           │  CUDA-GL interop (zero copy)
┌──────────────────────────▼───────────────────────────────────┐
│                     GPU  (Device)                            │
│  ┌────────────────┐  ┌────────────────┐  ┌───────────────┐  │
│  │ Force Kernel   │  │ Velocity Verlet│  │ OpenGL Render │  │
│  │ (N²/BH/Hash)  │─▶│ Integration    │─▶│ (point sprite)│  │
│  └────────────────┘  └────────────────┘  └───────────────┘  │
└──────────────────────────────────────────────────────────────┘
```

## Documentation

Explore our comprehensive documentation to get started:

- **[Getting Started](docs-content/setup/getting-started.md)** — Installation and first run
- **[Architecture](docs-content/architecture/architecture.md)** — System design and components
- **[Algorithms](docs-content/architecture/algorithms.md)** — Force calculation algorithms explained
- **[Performance Guide](docs-content/architecture/performance.md)** — Optimization strategies
- **[API Reference](docs-content/architecture/api.md)** — Complete API documentation

---

**Ready to dive in?** [View Documentation](docs.md){: .btn .btn-primary .mr-2 } [GitHub Repository](https://github.com/LessUp/n-body){: .btn }
