---
layout: default
title: Examples
parent: Documentation
nav_order: 7
---

# Examples

This directory contains focused example programs for the major n-body workflows.

## Available Programs

| Program | Purpose |
|---------|---------|
| [`example_basic.cpp`](example_basic.cpp) | Minimal end-to-end simulation |
| [`example_force_methods.cpp`](example_force_methods.cpp) | Compare force algorithms |
| [`example_custom_distribution.cpp`](example_custom_distribution.cpp) | Build custom particle layouts |
| [`example_energy_conservation.cpp`](example_energy_conservation.cpp) | Inspect integrator stability |

## Build

Examples are built by default through the canonical CMake path:

```bash
./scripts/build.sh
```

Or manually:

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DNBODY_BUILD_EXAMPLES=ON
cmake --build . -j"$(nproc)"
```

Generated executables live in `build/`:

```bash
./build/example_basic
./build/example_force_methods
./build/example_custom_distribution
./build/example_energy_conservation
```

## What Each Example Covers

### `example_basic.cpp`

- initialize a `ParticleSystem`
- run a basic simulation loop
- save and load state
- inspect total energy

### `example_force_methods.cpp`

- compare Direct N², Barnes-Hut, and Spatial Hash
- switch algorithms at runtime
- inspect performance trade-offs

### `example_custom_distribution.cpp`

- create custom initial particle distributions
- write particle data directly
- experiment with non-default setups

### `example_energy_conservation.cpp`

- track energy drift over time
- inspect timestep sensitivity
- evaluate integrator stability

## Related Docs

- [Getting Started](../docs/setup/getting-started.md)
- [Architecture](../docs/architecture/architecture.md)
- [Algorithms](../docs/architecture/algorithms.md)
- [Performance](../docs/architecture/performance.md)
