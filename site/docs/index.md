---
title: Documentation
---

# Documentation

Welcome to the N-Body Particle Simulation documentation. This guide covers everything from getting started to advanced optimization techniques.

## Quick Navigation

### Getting Started
New to the project? Start here:
- **[Getting Started]({% link docs/getting-started.md %})** — Installation, build instructions, and first run

### Architecture & Design
Understand the system design and components:
- **[Architecture Overview]({% link docs/architecture.md %})** — System design, components, and data flow
- **[Algorithms]({% link docs/algorithms.md %})** — Force calculation algorithms explained
- **[API Reference]({% link docs/api.md %})** — Complete API documentation
- **[Performance Guide]({% link docs/performance.md %})** — Optimization strategies and profiling

## What is N-Body Simulation?

N-Body simulation is a computational method that models the motion of particles under the influence of physical forces, typically gravity. This project provides a high-performance GPU-accelerated implementation supporting millions of particles with real-time visualization.

## System Capabilities

| Feature | Capability |
|---------|------------|
| Max Particles | 10+ million |
| GPU Acceleration | CUDA 11.0+ |
| Real-time Rendering | OpenGL 3.3+ |
| Algorithms | 3 (Direct N², Barnes-Hut, Spatial Hash) |
| Integration | Velocity Verlet (Symplectic) |

## External Resources

- [Main README](https://github.com/LessUp/n-body#readme) — Project overview
- [Changelog]({% link changelog/index.md %}) — Version history
- [Examples](https://github.com/LessUp/n-body/tree/main/examples) — Code samples
- [Contributing Guide](https://github.com/LessUp/n-body/blob/main/CONTRIBUTING.md) — How to contribute

---

Need help? [Open an Issue](https://github.com/LessUp/n-body/issues) or [View Discussions](https://github.com/LessUp/n-body/discussions)
