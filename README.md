# N-Body Particle Simulation System

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![CUDA](https://img.shields.io/badge/CUDA-11.0+-76B900?logo=nvidia&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3+-5586A4?logo=opengl&logoColor=white)

English | [简体中文](README.zh-CN.md)

Large-scale N-Body particle simulation system supporting million-particle GPU parallel computation and real-time visualization.

## Features

- **High-Performance GPU Computing** — CUDA parallel computation, million-particle real-time simulation
- **Multiple Force Algorithms**:
  - Direct N² — O(N²) exact computation
  - Barnes-Hut — O(N log N) tree approximation
  - Spatial Hash — O(N) grid-based neighbor search
- **Real-Time Visualization** — CUDA-OpenGL interop, no CPU-GPU data transfer
- **Multi-Integrator** — Euler, Velocity Verlet, Leapfrog
- **Interactive Control** — Camera rotation/zoom, real-time parameter adjustment

## Requirements

- CUDA Toolkit 11.0+, CMake 3.18+, C++17
- OpenGL 3.3+, GLFW, GLEW
- GPU CC 7.0+

## Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

## Usage

```bash
# Default simulation
./n_body_sim

# Custom parameters
./n_body_sim --particles 100000 --algorithm barnes-hut --dt 0.001
```

## Performance

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|-----------|----------|------------|--------------|
| 10K | 60 FPS | 120+ FPS | 120+ FPS |
| 100K | ~1 FPS | 60+ FPS | 90+ FPS |
| 1M | — | 15+ FPS | 30+ FPS |

*Tested on RTX 3060*

## License

MIT License
