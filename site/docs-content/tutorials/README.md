---
layout: default
title: Tutorials
parent: Documentation
nav_order: 3
has_children: true
---

# Tutorials & Examples

This directory contains tutorials and code examples for the N-Body Particle Simulation System.

## 📚 Tutorial List

| Tutorial | Description |
|----------|-------------|
| Basic Usage | Minimal simulation setup and execution |
| Algorithm Comparison | Performance comparison of different force calculation algorithms |
| Custom Distribution | Custom particle initial conditions |
| Energy Monitoring | Monitor energy conservation during simulation |

## 🔗 Related Resources

- [Getting Started](../setup/getting-started.md) — Installation and first run
- [Architecture Overview](../architecture/architecture.md) — System design
- [Example Code](https://github.com/LessUp/n-body/tree/main/examples) — Complete code examples

## 📖 Code Examples

### Basic Simulation

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 100000;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;
    
    ParticleSystem system;
    system.initialize(config);
    
    for (int i = 0; i < 1000; ++i) {
        system.update(system.getTimeStep());
    }
    
    return 0;
}
```

### Energy Monitoring

```cpp
float ke = system.computeKineticEnergy();
float pe = system.computePotentialEnergy();
float total = system.computeTotalEnergy();
std::cout << "Energy: KE=" << ke << " PE=" << pe << " Total=" << total << std::endl;
```

### Save and Load State

```cpp
// Save
system.saveState("checkpoint.nbody");

// Load
system.loadState("checkpoint.nbody");
```
