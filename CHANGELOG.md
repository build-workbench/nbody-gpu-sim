# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

---

## [2.1.0] - 2026-04-27

### Changed
- Version synchronized to 2.1.0 across CMakeLists.txt and openspec.yaml
- CUDA stack size increased from 128 to 256 for improved handling of non-uniform particle distributions
- Changelog consolidated from bilingual changelog/ directory to single CHANGELOG.md
- Removed empty `src/core/force_calculator.cpp` (implementation lives in CUDA files)
- Fixed CODEOWNERS case sensitivity: `@Lessup` → `@LessUp`
- Optimized CI/CD paths filtering to prevent unnecessary workflow triggers

### Removed
- `changelog/` directory (consolidated into this file)

---

## [2.0.0] - 2026-03-13

### Highlights

| Category | Improvement |
|----------|-------------|
| 🐛 **Critical Bugs** | 2 fixed (bounding box handling, memory safety) |
| ⚡ **Performance** | ~15-30% faster in release builds |
| 🔧 **Build System** | Modern CMake, better IDE support |
| 🔄 **CI/CD** | Reliable CPU-safe format checking |

### Fixed

#### Critical: Barnes-Hut Bounding Box with Negative Coordinates
- **Component**: `BarnesHutTree::computeBoundingBox()`
- **Problem**: `atomicMin` with `reinterpret_cast<int*>` failed for negative floats
- **Solution**: CAS-based atomic float operations for all sign combinations

#### Critical: SpatialHashGrid Uninitialized Pointers
- **Component**: `SpatialHashGrid` constructor
- **Problem**: Device pointers not initialized to `nullptr`
- **Solution**: All device pointers initialized in constructor

### Changed

#### Performance Improvements
- **GPU-Based Bounding Box**: Eliminated CPU↔GPU roundtrip
  - 1M particles: 8.2ms → 0.3ms (27× faster)
  - 10M particles: 82ms → 2.1ms (39× faster)
- **Scratch Buffer Reuse**: Eliminated per-call `cudaMalloc`/`cudaFree`
- **Conditional Kernel Synchronization**: Async error checking in Release builds

#### Build System
- CMake modernization with `CMAKE_EXPORT_COMPILE_COMMANDS`
- Automatic CUDA architecture detection
- Cross-platform compiler warnings

#### CI/CD
- Restructured to CPU-safe checks that always pass
- Format checking with clang-format
- Documentation consistency verification

### Added
- `atomicMinFloat`/`atomicMaxFloat` for correct float atomic operations
- Persistent scratch buffer for energy calculations
- `ensureScratchBuffer()` method for buffer management

---

## [1.0.0] - 2025-02-13

### Added

#### Force Calculation Algorithms
- **Direct N² (O(N²))**: Exact pairwise force calculation with shared memory tiling
- **Barnes-Hut (O(N log N))**: Octree-based spatial decomposition with Morton code sorting
- **Spatial Hash (O(N))**: Uniform grid for short-range forces

#### Time Integration
- Velocity Verlet symplectic integrator with second-order accuracy

#### Rendering System
- OpenGL 3.3+ point sprite rendering
- CUDA-OpenGL zero-copy interop
- Real-time camera controls (orbit, zoom)

#### Particle Distributions
- Uniform, Spherical, Disk (with initial rotation for galaxy simulation)

#### Project Infrastructure
- CMake 3.18+ build system
- Google Test integration
- RapidCheck property-based testing
- MIT License
- Bilingual documentation (English & Chinese)

### Performance (RTX 3080)

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|-----------|-----------|------------|--------------|
| 100,000 | ~8 FPS | 60+ FPS | 60+ FPS |
| 500,000 | <1 FPS | ~45 FPS | 60+ FPS |
| 1,000,000 | — | ~25 FPS | 60+ FPS |

---

## Version History Summary

| Version | Date | Type | Key Changes |
|---------|------|------|-------------|
| 2.1.0 | 2026-04-27 | Refactor | Project cleanup, CI optimization, bug fixes |
| 2.0.0 | 2026-03-13 | Major | Critical bug fixes, performance improvements |
| 1.0.0 | 2025-02-13 | Initial | Complete N-body simulation framework |

---

[Unreleased]: https://github.com/LessUp/n-body/compare/v2.1.0...HEAD
[2.1.0]: https://github.com/LessUp/n-body/compare/v2.0.0...v2.1.0
[2.0.0]: https://github.com/LessUp/n-body/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/LessUp/n-body/releases/tag/v1.0.0
