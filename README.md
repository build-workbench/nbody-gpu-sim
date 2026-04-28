# n-body

High-performance GPU N-body simulation with real-time CUDA/OpenGL visualization.

[GitHub Pages](https://lessup.github.io/n-body/) · [Getting Started](docs/setup/getting-started.md) · [Examples](examples/) · [OpenSpec](openspec/specs/)

## Why this project

This project combines three force-computation strategies with a single simulation/runtime API:

- **Direct N²** for exact pairwise reference results
- **Barnes-Hut** for scalable long-range approximation
- **Spatial Hash** for efficient short-range interaction
- **Velocity Verlet** for stable symplectic integration
- **CUDA/OpenGL interop** for zero-copy visualization

The goal is not just to render particles quickly, but to keep the simulation architecture understandable, testable, and easy to compare across algorithms.

## Technical Highlights

| Area | What it provides |
|------|------------------|
| Compute | CUDA kernels for force evaluation and integration |
| Algorithms | Direct N², Barnes-Hut, Spatial Hash |
| Rendering | OpenGL renderer with CUDA/OpenGL interop |
| Architecture | `ParticleSystem` facade + `ForceCalculator` strategy |
| Quality | GoogleTest + RapidCheck, OpenSpec-driven workflow |

## Algorithm Guide

| Algorithm | Complexity | Best fit |
|-----------|------------|----------|
| Direct N² | O(N²) | Small systems, reference validation |
| Barnes-Hut | O(N log N) | Large gravitational systems |
| Spatial Hash | O(N) | Short-range interaction workloads |

## Quick Start

### Requirements

- NVIDIA GPU with CUDA support
- CUDA Toolkit 11+
- CMake 3.18+
- OpenGL, GLFW, GLEW, GLM
- For a headless core-only validation path, disable CUDA/rendering; headless observability tests and benchmarks still work.

### Build

```bash
./scripts/build.sh
```

When CUDA is unavailable, the script now falls back to a headless core-only build and still produces the core library, headless observability tests, and the benchmark executable. Rendered app surfaces and examples remain disabled.

Manual path:

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j"$(nproc)"
```

Manual headless core-only path:

```bash
mkdir -p build/headless
cd build/headless
cmake ../.. \
  -DCMAKE_BUILD_TYPE=Release \
  -DNBODY_ENABLE_RENDERING=OFF \
  -DNBODY_ENABLE_CUDA=OFF \
  -DNBODY_BUILD_TESTS=ON \
  -DNBODY_BUILD_BENCHMARKS=ON \
  -DNBODY_BUILD_EXAMPLES=OFF
cmake --build . -j"$(nproc)"
```

### Run

```bash
./build/nbody_sim
./build/nbody_sim 100000
```

### Test

```bash
./scripts/test.sh
```

`./scripts/test.sh` only applies to CUDA-enabled builds that actually produced `nbody_tests`. Headless core-only builds now explain why the suite is unavailable instead of pointing at a missing binary.

### Benchmark

```bash
./scripts/benchmark.sh
./scripts/benchmark.sh serialization.round_trip build/benchmark-results.json
```

Set `NBODY_BENCHMARK_PARTICLES`, `NBODY_BENCHMARK_ITERATIONS`, or enable profiling with `-DNBODY_ENABLE_PROFILING=ON` to tune benchmark runs and phase timing output.

## Project Layout

| Path | Purpose |
|------|---------|
| `include/nbody/` | Public headers |
| `src/` | Core, CUDA, rendering, utilities |
| `tests/` | Unit and property-based tests |
| `examples/` | Example programs and usage patterns |
| `docs/` | Canonical repository-local documentation |
| `site/` | GitHub Pages showcase |
| `openspec/specs/` | Active specifications |
| `openspec/changes/` | Active proposals and implementation tasks |

## Canonical Documentation

- [Getting Started](docs/setup/getting-started.md)
- [Architecture](docs/architecture/architecture.md)
- [Algorithms](docs/architecture/algorithms.md)
- [API Reference](docs/architecture/api.md)
- [Performance](docs/architecture/performance.md)
- [Contributing](CONTRIBUTING.md)

## OpenSpec Workflow

This repository is governed by OpenSpec.

1. Read the relevant files in [`openspec/specs/`](openspec/specs/).
2. Create or update an OpenSpec change in [`openspec/changes/`](openspec/changes/) before implementing behavior or workflow changes.
3. Implement from the change task list.
4. Use `/review` before finalizing major structural or governance refactors.

Active capability specs:

- [simulation-core](openspec/specs/simulation-core.md)
- [force-computation](openspec/specs/force-computation.md)
- [visualization](openspec/specs/visualization.md)
- [simulation-control](openspec/specs/simulation-control.md)
- [quality-attributes](openspec/specs/quality-attributes.md)
- [repository-governance](openspec/specs/repository-governance.md)

## Examples

- [`example_basic.cpp`](examples/example_basic.cpp)
- [`example_force_methods.cpp`](examples/example_force_methods.cpp)
- [`example_custom_distribution.cpp`](examples/example_custom_distribution.cpp)
- [`example_energy_conservation.cpp`](examples/example_energy_conservation.cpp)

## Development Notes

- Canonical build path: CMake + `scripts/build.sh`
- Canonical LSP baseline: `clangd` + `compile_commands.json`
- Canonical assistant guidance: [AGENTS.md](AGENTS.md), [CLAUDE.md](CLAUDE.md), [.github/copilot-instructions.md](.github/copilot-instructions.md)

## License

[MIT](LICENSE)
