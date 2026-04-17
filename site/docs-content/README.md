# Documentation

Welcome to the N-Body Particle Simulation documentation.

## 📚 Documentation Structure

| Directory | Purpose |
|-----------|---------|
| [`setup/`](./setup/) | Environment setup, build instructions, troubleshooting |
| [`tutorials/`](./tutorials/) | User guides, tutorials, and usage examples |
| [`architecture/`](./architecture/) | System design, algorithms, API reference, performance |
| [`assets/`](./assets/) | Images, diagrams, and other static resources |

## 🚀 Quick Navigation

### Getting Started
- [Setup Guide](./setup/getting-started.md) — Installation, build, and first run

### Architecture & Design
- [Architecture Overview](./architecture/architecture.md) — System design, components, and data flow
- [Algorithms](./architecture/algorithms.md) — Force calculation algorithms explained
- [API Reference](./architecture/api.md) — Complete API documentation
- [Performance Guide](./architecture/performance.md) — Optimization strategies and profiling

### Specs (Single Source of Truth)
- [Product Spec](../specs/product/n-body-simulation.md) — Requirements and acceptance criteria
- [Architecture RFC](../specs/rfc/0001-core-architecture.md) — Technical design decisions

## 🌐 Available Languages

| Language | Documentation |
|----------|---------------|
| 🇺🇸 English | [This directory](./) |
| 🇨🇳 简体中文 | [中文文档](./zh-CN/) |

## 🎯 What is N-Body Simulation?

N-Body simulation is a computational method that models the motion of particles under the influence of physical forces, typically gravity. This project provides a high-performance GPU-accelerated implementation supporting millions of particles with real-time visualization.

## 📊 System Capabilities

| Feature | Capability |
|---------|------------|
| Max Particles | 10+ million |
| GPU Acceleration | CUDA 11.0+ |
| Real-time Rendering | OpenGL 3.3+ |
| Algorithms | 3 (Direct N², Barnes-Hut, Spatial Hash) |
| Integration | Velocity Verlet (Symplectic) |

## 🔗 External Resources

- [Main README](../README.md)
- [Changelog](../CHANGELOG.md)
- [Examples](../examples/)
- [Contributing Guide](../CONTRIBUTING.md)
