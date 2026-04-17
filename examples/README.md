---
layout: default
title: Examples
parent: Documentation
nav_order: 7
---

# Examples

This directory contains example code demonstrating various features of the N-Body Particle Simulation System.

## Examples Overview

| File | Description |
|------|-------------|
| [example_basic.cpp](example_basic.cpp) | Basic simulation setup and usage |
| [example_force_methods.cpp](example_force_methods.cpp) | Comparing different force algorithms |
| [example_custom_distribution.cpp](example_custom_distribution.cpp) | Creating custom particle distributions |
| [example_energy_conservation.cpp](example_energy_conservation.cpp) | Analyzing energy conservation |

## Building Examples

Examples are built as part of the main project:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Example Descriptions

### 1. Basic Simulation (`example_basic.cpp`)

Demonstrates the minimal setup required to run an N-Body simulation:

- Configure simulation parameters
- Initialize the particle system
- Run a basic simulation loop
- Save and load simulation state

```bash
./example_basic
```

**Key concepts:**
- `SimulationConfig` for parameter configuration
- `ParticleSystem::initialize()` for setup
- `ParticleSystem::update()` for simulation steps
- `ParticleSystem::saveState()` / `loadState()` for persistence

### 2. Force Methods Comparison (`example_force_methods.cpp`)

Compares the three force calculation algorithms:

- **Direct N²**: O(N²) exact calculation
- **Barnes-Hut**: O(N log N) tree-based approximation
- **Spatial Hash**: O(N) for short-range forces

```bash
./example_force_methods
```

**Key concepts:**
- Switching algorithms at runtime with `setForceMethod()`
- Performance measurement and comparison
- Understanding algorithm trade-offs

### 3. Custom Distribution (`example_custom_distribution.cpp`)

Shows how to create custom particle distributions:

- Creating a spiral galaxy pattern
- Setting up initial velocities for stable orbits
- Manual particle initialization

```bash
./example_custom_distribution
```

**Key concepts:**
- Direct particle data manipulation
- `ParticleDataManager` for memory operations
- `ParticleInitializer` utilities

### 4. Energy Conservation (`example_energy_conservation.cpp`)

Analyzes the energy conservation properties of the Velocity Verlet integrator:

- Computing kinetic and potential energy
- Tracking energy drift over time
- Understanding symplectic integrator behavior

```bash
./example_energy_conservation
```

**Key concepts:**
- Energy computation methods
- Time step impact on accuracy
- Symplectic integration properties

## Quick Start Template

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    // 1. Configure
    SimulationConfig config;
    config.particle_count = 10000;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;

    // 2. Initialize
    ParticleSystem system;
    system.initialize(config);

    // 3. Simulate
    for (int i = 0; i < 1000; i++) {
        system.update(system.getTimeStep());
    }

    // 4. Analyze
    std::cout << "Energy: " << system.computeTotalEnergy() << "\n";

    return 0;
}
```

## Common Patterns

### Switching Algorithms

```cpp
// Switch to Barnes-Hut for large systems
if (particle_count > 50000) {
    system.setForceMethod(ForceMethod::BARNES_HUT);
    system.setBarnesHutTheta(0.5f);
}

// Switch to Direct N² for accuracy
if (need_exact_results) {
    system.setForceMethod(ForceMethod::DIRECT_N2);
}

// Switch to Spatial Hash for short-range forces
system.setForceMethod(ForceMethod::SPATIAL_HASH);
system.setSpatialHashCutoff(2.0f);
```

### Pause and Resume

```cpp
// Pause simulation
system.pause();

// Analyze or modify state
auto state = system.getState();
// ... modify state ...
system.setState(state);

// Resume simulation
system.resume();
```

### State Persistence

```cpp
// Save checkpoint
system.saveState("checkpoint.nbody");

// Later, load checkpoint
ParticleSystem loaded_system;
loaded_system.loadState("checkpoint.nbody");

// Continue from checkpoint
loaded_system.update(loaded_system.getTimeStep());
```

### Error Handling

```cpp
try {
    ParticleSystem system;
    system.initialize(config);

    // ... simulation ...

} catch (const CudaException& e) {
    std::cerr << "CUDA Error: " << e.what() << "\n";
    std::cerr << "  File: " << e.getFile() << "\n";
    std::cerr << "  Line: " << e.getLine() << "\n";

} catch (const ValidationException& e) {
    std::cerr << "Validation Error: " << e.what() << "\n";

} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
}
```

## Further Reading

- [API Reference](../docs/API.md) - Complete API documentation
- [Architecture](../docs/ARCHITECTURE.md) - System design overview
- [Algorithms](../docs/ALGORITHMS.md) - Algorithm explanations
- [Performance Guide](../docs/PERFORMANCE.md) - Optimization tips
