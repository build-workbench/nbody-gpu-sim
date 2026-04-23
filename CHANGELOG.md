# Changelog

**All notable changes to this project are documented in the [changelog/](./changelog/) directory.**

This repository follows [Semantic Versioning](https://semver.org/) and [Keep a Changelog](https://keepachangelog.com/) format.

## Quick Links

| Language | Version History |
|----------|-----------------|
| 🇺🇸 English | [changelog/en/](./changelog/en/) |
| 🇨🇳 中文 | [changelog/zh-CN/](./changelog/zh-CN/) |

### Latest Release

- **v2.0.0** - Critical bug fixes and performance optimizations
- **v1.0.0** - Initial release with core algorithms

---

**Note**: This file serves as a navigation point. Detailed release notes are in `changelog/` directory.

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
