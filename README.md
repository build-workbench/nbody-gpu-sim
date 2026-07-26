# n-body

[![Build](https://github.com/AICL-Lab/n-body/actions/workflows/ci.yml/badge.svg)](https://github.com/AICL-Lab/n-body/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

**Million-Particle GPU Physics Engine** — CUDA-accelerated N-body simulation with real-time OpenGL visualization and three force algorithms.

[GitHub Pages](https://aicl-lab.github.io/n-body/) · [Examples](examples/)

## Algorithms

| Algorithm | Complexity | Best fit |
|-----------|------------|----------|
| Direct N² | O(N²) | Small systems, reference validation |
| Barnes-Hut | O(N log N) | Large gravitational systems |
| Spatial Hash | O(N) | Short-range interaction |

## Performance

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|-----------|-----------|------------|--------------|
| 10K | 60 FPS | 120 FPS | 120 FPS |
| 100K | 10 FPS | 60 FPS | 90 FPS |
| 1M | 1 FPS | 25 FPS | 60 FPS |

*Benchmarks on NVIDIA RTX 3080*

## Quick Start

Requirements: NVIDIA GPU, CUDA Toolkit 11+, CMake 3.18+, OpenGL/GLFW/GLEW/GLM.

```bash
./scripts/build.sh
./build/nbody_sim 100000
```

Test and benchmark:

```bash
./scripts/test.sh
./scripts/benchmark.sh
```

## Project Layout

| Path | Purpose |
|------|---------|
| `include/nbody/` | Public headers |
| `src/` | Core, CUDA, rendering, utilities |
| `tests/` | Unit and property-based tests |
| `examples/` | Example programs |
| `site/` | GitHub Pages showcase |

## Development

- Build: CMake + `scripts/build.sh`
- LSP: `clangd` + `compile_commands.json`
- AI assistant guidance: [AGENTS.md](AGENTS.md)

## License

[MIT](LICENSE)
