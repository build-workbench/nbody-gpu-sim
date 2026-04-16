# Changelog

All notable changes to the N-Body Particle Simulation project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- Bilingual documentation (English & Chinese)
- Reorganized docs structure with `en/` and `zh-CN/` directories
- Reorganized changelog structure with `en/` and `zh-CN/` directories
- Professional release notes template

### Changed
- Improved documentation structure and navigation
- Enhanced README with better badges and formatting

---

## [2.0.0] - 2026-03-13

### 🐛 Fixed (Critical)

#### Barnes-Hut Bounding Box with Negative Coordinates
- **Problem**: `atomicMin` with `reinterpret_cast<int*>` fails for negative floats
- **Impact**: Incorrect octree construction with negative particle coordinates
- **Solution**: Implemented CAS-based `atomicMinFloat`/`atomicMaxFloat`
- **Files**: `src/cuda/force_barnes_hut.cu`

#### SpatialHashGrid Uninitialized Pointers
- **Problem**: Device pointers not initialized to `nullptr`
- **Impact**: Undefined behavior if destructor called before `build()`
- **Solution**: All device pointers initialized in constructor
- **Files**: `src/cuda/force_spatial_hash.cu`

### ⚡ Performance

#### GPU-Based Bounding Box Computation
| Particles | Before | After | Speedup |
|-----------|--------|-------|---------|
| 100K | 0.8 ms | 0.05 ms | 16× |
| 1M | 8.2 ms | 0.3 ms | 27× |
| 10M | 82 ms | 2.1 ms | 39× |

#### Scratch Buffer Reuse
- Eliminated `cudaMalloc`/`cudaFree` per energy calculation
- Added persistent `d_scratch_` buffer

#### Conditional Kernel Synchronization
- Release builds: async error checking (`cudaGetLastError()`)
- Debug builds: sync error checking (`cudaDeviceSynchronize()`)

### 🔧 Build System

- Added CMake version specification (2.0.0)
- Enabled `CMAKE_EXPORT_COMPILE_COMMANDS` for IDE support
- Added CUDA architecture auto-detection
- Modernized to `target_include_directories`
- Added cross-platform compiler warnings

### 🔄 CI/CD

- Replaced GPU-dependent tests with CPU-safe checks
- Added format checking with clang-format 17
- Added documentation consistency verification

---

## [1.0.0] - 2025-02-13

### ✨ Added

#### Force Calculation Algorithms
- **Direct N²**: O(N²) exact computation with shared memory tiling
- **Barnes-Hut**: O(N log N) tree approximation with configurable θ
- **Spatial Hash**: O(N) grid-based for short-range forces

#### Time Integration
- Velocity Verlet symplectic integrator
- Second-order accuracy, energy conserving

#### Rendering
- OpenGL 3.3+ point sprite visualization
- CUDA-OpenGL zero-copy interop
- Real-time orbit camera controls

#### Particle Distributions
- Uniform (box), Spherical, Disk (galaxy)

#### Infrastructure
- CMake 3.18+ build system
- Google Test + RapidCheck testing
- MIT License
- Documentation suite

---

## Migration Guides

### Upgrading from 1.0.0 to 2.0.0

**No breaking API changes** — all existing code compiles and runs correctly.

Recommended clean rebuild:
```bash
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

---

[Detailed release notes →](./changelog/)
