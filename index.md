---
layout: default
title: N-Body Particle Simulation
description: Million-particle GPU simulation with real-time visualization
nav_order: 0
---

# N-Body Particle Simulation

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![CUDA](https://img.shields.io/badge/CUDA-11.0+-76B900?logo=nvidia&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3+-5586A4?logo=opengl&logoColor=white)

A million-particle GPU simulation system with real-time visualization. Implements three force-calculation algorithms — Direct N², Barnes-Hut, and Spatial Hash — running entirely on the GPU via CUDA, with zero-copy CUDA-OpenGL interop rendering.

## Performance

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|-----------|-----------|------------|--------------|
| 10 K      | 60+ FPS   | 60+ FPS    | 60+ FPS      |
| 100 K     | ~10 FPS   | 60+ FPS    | 60+ FPS      |
| **1 M**   | <1 FPS    | ~30 FPS    | **60+ FPS**  |

*Tested on NVIDIA RTX 3080, CUDA 11.8*

## Key Features

- **Million-particle real-time simulation** — CUDA parallel physics with three switchable algorithms
- **Zero-copy rendering** — CUDA-OpenGL interop eliminates CPU↔GPU data transfer
- **Symplectic integrator** — Velocity Verlet ensures long-term energy conservation
- **Interactive controls** — Camera orbit, zoom, reset, and force-algorithm hot-swap at runtime

## Force Algorithms

| Algorithm | Complexity | Best For |
|-----------|------------|----------|
| **Direct N²** | O(N²) | Small systems, exact results |
| **Barnes-Hut** | O(N log N) | Large-scale gravitational simulation |
| **Spatial Hash** | O(N) | Short-range forces (molecular dynamics) |

**Barnes-Hut** builds an octree each frame, approximating distant particle clusters as single mass points (controlled by θ parameter). **Spatial Hash** bins particles into a uniform grid so only neighboring cells interact — ideal for short-range potentials.

## Architecture

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

## Quick Start

```bash
# Dependencies (Ubuntu)
sudo apt-get install -y cmake libglfw3-dev libglew-dev libglm-dev

# Build
mkdir build && cd build
cmake .. && make -j$(nproc)

# Run (default 10K particles)
./nbody_sim

# Million-particle simulation
./nbody_sim 1000000
```

## Controls

| Key | Action |
|-----|--------|
| `1` / `2` / `3` | Switch Direct / Barnes-Hut / Spatial Hash |
| `Space` | Pause / resume |
| `R` | Reset simulation |
| `C` | Reset camera |
| `Esc` | Quit |
| Mouse drag | Rotate view |
| Scroll wheel | Zoom |

## Documentation

### Getting Started

- **[Getting Started Guide](docs/GETTING_STARTED.html)** — Setup and usage instructions
- **[Examples](examples/README.md)** — Code examples for common scenarios

### Reference

- **[API Reference](docs/API.html)** — Complete API documentation
- **[Architecture](docs/ARCHITECTURE.html)** — System design and components
- **[Algorithms](docs/ALGORITHMS.html)** — Algorithm explanations
- **[Performance Guide](docs/PERFORMANCE.html)** — Optimization strategies

## Tech Stack

| Category | Technology |
|----------|------------|
| Language | CUDA C++17 |
| Rendering | OpenGL 3.3+, GLFW, GLEW, GLM |
| Build | CMake 3.18+ |
| GPU | Compute Capability 7.5+ (Turing → Hopper) |
| Testing | Google Test + RapidCheck |

## References

1. Barnes & Hut (1986). *A hierarchical O(N log N) force-calculation algorithm.* Nature 324.
2. Nyland, Harris & Prins (2007). *Fast N-body simulation with CUDA.* GPU Gems 3.
3. Green (2010). *Particle simulation using CUDA.* NVIDIA Whitepaper.

---

[View on GitHub](https://github.com/LessUp/n-body)
