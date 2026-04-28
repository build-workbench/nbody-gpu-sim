---
layout: docs
lang: en
title: API Reference
description: Complete API documentation for N-Body Particle Simulation.
---

# API Reference

Complete API documentation for N-Body Particle Simulation.

---
layout: docs
lang: en

## Quick Example

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    // Configure and initialize
    SimulationConfig config;
    config.particle_count = 100'000;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;
    
    ParticleSystem system;
    system.initialize(config);
    
    // Run simulation
    for (int step = 0; step < 10000; ++step) {
        system.update(config.dt);
    }
    
    return 0;
}
```

---
layout: docs
lang: en

## SimulationConfig

Configuration structure for simulation setup.

```cpp
struct SimulationConfig {
    // Particle count
    size_t particle_count = 10000;
    
    // Force calculation method
    ForceMethod force_method = ForceMethod::BARNES_HUT;
    
    // Time step (seconds)
    float dt = 0.001f;
    
    // Gravitational constant
    float G = 6.674e-11f;
    
    // Softening parameter (prevents singularities)
    float softening = 0.01f;
    
    // Barnes-Hut opening angle
    float theta = 0.5f;
    
    // Spatial hash cell size
    float cell_size = 1.0f;
    
    // Initial particle distribution
    InitDistribution distribution = InitDistribution::UNIFORM_SPHERE;
    
    // Random seed (0 = random)
    unsigned int seed = 0;
};
```

### ForceMethod Enum

```cpp
enum class ForceMethod {
    DIRECT,         // O(N²) exact calculation
    BARNES_HUT,     // O(N log N) tree-based
    SPATIAL_HASH    // O(N) grid-based
};
```

### InitDistribution Enum

```cpp
enum class InitDistribution {
    UNIFORM_SPHERE,     // Uniform random in sphere
    UNIFORM_CUBE,       // Uniform random in cube
    SHELL,              // Uniform on spherical shell
    GAUSSIAN,           // Gaussian (normal) distribution
    DISC                // Uniform in disc (2D)
};
```

---
layout: docs
lang: en

## ParticleSystem

Main orchestrator class managing the simulation.

### Methods

#### initialize()

```cpp
bool initialize(const SimulationConfig& config);
```

Initializes the simulation with given configuration.

**Returns:** `true` on success, `false` on failure.

#### update()

```cpp
void update(float dt);
```

Performs one simulation step (force computation + integration).

#### setForceMethod()

```cpp
void setForceMethod(ForceMethod method);
```

Switches force calculation algorithm at runtime.

#### getTimeStep()

```cpp
float getTimeStep() const;
```

Returns configured time step.

#### saveState() / loadState()

```cpp
bool saveState(const std::string& filename) const;
bool loadState(const std::string& filename);
```

Save/load simulation state to/from file.

---
layout: docs
lang: en

## ForceCalculator

Abstract base class for force calculation algorithms.

```cpp
class ForceCalculator {
public:
    virtual ~ForceCalculator() = default;
    
    // Compute forces for all particles
    virtual void computeForces(ParticleData* particles) = 0;
    
    // Get algorithm name
    virtual const char* getName() const = 0;
    
    // Get time complexity description
    virtual const char* getComplexity() const = 0;
};
```

---
layout: docs
lang: en

## Error Handling

Errors are handled through return values and optional error callbacks:

```cpp
// Check initialization
if (!system.initialize(config)) {
    std::cerr << "Failed to initialize system\n";
    return 1;
}

// CUDA errors are automatically checked in debug builds
```
