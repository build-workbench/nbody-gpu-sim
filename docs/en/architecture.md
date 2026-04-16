---
layout: default
title: Architecture
parent: Documentation
nav_order: 2
---

# Architecture Overview

This document describes the system architecture, component interactions, and design patterns of the N-Body Particle Simulation System.

---

## 📐 High-Level Architecture

### System Layers

```
┌─────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                     │
│  • Window Management (GLFW)                             │
│  • Input Processing                                     │
│  • Main Event Loop                                      │
├─────────────────────────────────────────────────────────┤
│                   SIMULATION LAYER                       │
│  • ParticleSystem (Orchestrator)                        │
│  • ForceCalculator (Strategy Pattern)                   │
│  • Integrator (Velocity Verlet)                         │
│  • CudaGLInterop (CUDA-OpenGL Bridge)                   │
├─────────────────────────────────────────────────────────┤
│                   RENDERING LAYER                        │
│  • Renderer (OpenGL)                                    │
│  • Camera (Orbit Controls)                              │
│  • Shader Management                                    │
├─────────────────────────────────────────────────────────┤
│                    GPU MEMORY LAYER                      │
│  • ParticleData (SoA Layout)                            │
│  • Shared VBO (Zero-Copy Interop)                       │
│  • Acceleration Structures                              │
└─────────────────────────────────────────────────────────┘
```

---

## 🔧 Core Components

### 1. ParticleSystem

The central orchestrator managing all simulation components.

```cpp
class ParticleSystem {
public:
    void initialize(const SimulationConfig& config);
    void update(float dt);
    void pause(); / resume();
    void reset();
    
    void saveState(const std::string& filename);
    void loadState(const std::string& filename);
    
private:
    ParticleData d_particles_;                    // GPU data
    std::unique_ptr<ForceCalculator> force_calc_;
    std::unique_ptr<Integrator> integrator_;
    std::unique_ptr<CudaGLInterop> interop_;
};
```

**Responsibilities:**
- Initialize particle distributions
- Coordinate simulation steps
- Manage component lifecycle
- Handle state persistence

### 2. ForceCalculator (Strategy Pattern)

Abstract interface for force calculation algorithms.

```cpp
class ForceCalculator {
public:
    virtual void computeForces(ParticleData* d_particles) = 0;
    virtual ForceMethod getMethod() const = 0;
    
    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
};
```

**Implementations:**

| Class | Complexity | Best For |
|-------|------------|----------|
| `DirectForceCalculator` | O(N²) | Small systems, exact results |
| `BarnesHutCalculator` | O(N log N) | Large-scale gravity |
| `SpatialHashCalculator` | O(N) | Short-range forces |

### 3. Integrator

Velocity Verlet symplectic integrator.

```cpp
class Integrator {
public:
    void integrate(ParticleData* d_particles, 
                   ForceCalculator* force_calc, 
                   float dt);
    
private:
    void updatePositions(ParticleData* d_particles, float dt);
    void updateVelocities(ParticleData* d_particles, float dt);
    void storeOldAccelerations(ParticleData* d_particles);
};
```

**Algorithm:**
```
1. a_old = a(t)                           // Store old accelerations
2. x(t+dt) = x(t) + v(t)·dt + ½·a(t)·dt² // Update positions
3. a(t+dt) = F(x(t+dt))/m                // Compute new forces
4. v(t+dt) = v(t) + ½·(a_old + a(t+dt))·dt // Update velocities
```

### 4. CudaGLInterop

Enables zero-copy sharing between CUDA and OpenGL.

```cpp
class CudaGLInterop {
public:
    void initialize(size_t particle_count);
    float* mapPositionBuffer();     // Get CUDA device pointer
    void unmapPositionBuffer();     // Release for OpenGL
    void updatePositions(const ParticleData* d_particles);
    
    GLuint getPositionVBO() const;
    
private:
    GLuint position_vbo_;
    cudaGraphicsResource* cuda_vbo_;
};
```

**Data Flow:**
```
CUDA Compute → Map VBO → Copy Positions → Unmap VBO → OpenGL Render
     ↑                                                    │
     └─────────── Zero Copy (Same GPU Memory) ────────────┘
```

---

## 💾 Memory Architecture

### ParticleData (Structure of Arrays)

```cpp
struct ParticleData {
    // Position (3N floats)
    float* pos_x; float* pos_y; float* pos_z;
    
    // Velocity (3N floats)
    float* vel_x; float* vel_y; float* vel_z;
    
    // Acceleration (6N floats - current + old)
    float* acc_x; float* acc_y; float* acc_z;
    float* acc_old_x; float* acc_old_y; float* acc_old_z;
    
    // Mass (N floats)
    float* mass;
    
    size_t count;
};
```

**Memory per Particle:** 52 bytes (13 floats)

### Memory Budget (1M Particles)

| Component | Size | Description |
|-----------|------|-------------|
| ParticleData | 52 MB | Core particle state |
| Shared VBO | 12 MB | CUDA-OpenGL interop |
| Barnes-Hut Tree | ~100 MB | Octree structure |
| Spatial Hash Grid | ~20 MB | Cell lookups |
| **Total** | **~184 MB** | Maximum usage |

---

## 🔄 Data Flow

### Simulation Loop

```
┌────────────────────────────────────────┐
│  1. INPUT PROCESSING                   │
│     • Poll GLFW events                 │
│     • Update camera                    │
│     • Handle controls                  │
└────────────────┬───────────────────────┘
                 ▼
┌────────────────────────────────────────┐
│  2. PHYSICS UPDATE (if not paused)     │
│     • Velocity Verlet integration      │
│     • Force computation                │
└────────────────┬───────────────────────┘
                 ▼
┌────────────────────────────────────────┐
│  3. DATA TRANSFER                      │
│     • Update VBO via interop           │
│     • SoA → interleaved conversion     │
└────────────────┬───────────────────────┘
                 ▼
┌────────────────────────────────────────┐
│  4. RENDERING                          │
│     • Bind shaders/bindings            │
│     • Draw point sprites               │
│     • Swap buffers                     │
└────────────────┬───────────────────────┘
                 ▼
              [Repeat @ 60Hz]
```

### Algorithm-Specific Flows

#### Direct N²
```
ParticleData → [Load Tiles to Shared Memory] → [Compute All Pairs] → Forces
```

#### Barnes-Hut
```
ParticleData → [Bounding Box] → [Morton Codes] → [Sort] → [Build Tree] 
                                                    → [Traverse] → Forces
```

#### Spatial Hash
```
ParticleData → [Compute Cell IDs] → [Sort] → [Build Ranges] 
                                        → [Neighbor Search] → Forces
```

---

## 🧩 Design Patterns

### Strategy Pattern: ForceCalculator

Used to switch algorithms at runtime.

```cpp
// Interface
class ForceCalculator {
    virtual void computeForces(ParticleData* d_particles) = 0;
};

// Concrete strategies
class DirectForceCalculator : public ForceCalculator { };
class BarnesHutCalculator : public ForceCalculator { };
class SpatialHashCalculator : public ForceCalculator { };

// Usage
std::unique_ptr<ForceCalculator> force_calc;
force_calc = std::make_unique<BarnesHutCalculator>(theta);
force_calc->computeForces(d_particles);
```

### Bridge Pattern: CudaGLInterop

Decouples CUDA computation from OpenGL rendering.

```cpp
// CUDA side (writes)
float* d_vbo = interop.mapPositionBuffer();
kernel<<<grid, block>>>(d_particles, d_vbo);
interop.unmapPositionBuffer();

// OpenGL side (reads)
renderer.render(interop.getPositionVBO(), count);
```

### Facade Pattern: ParticleSystem

Simplifies complex subsystem interactions.

```cpp
// Client code doesn't need to know about:
// - Kernel launches
// - Memory management
// - Interop details

ParticleSystem system;
system.initialize(config);
system.update(dt);  // Simple, clean interface
```

---

## 🔌 Extension Points

### Adding a New Force Algorithm

1. **Create new class:**

```cpp
// include/nbody/my_force_calculator.hpp
class MyForceCalculator : public ForceCalculator {
public:
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override { return ForceMethod::MY_METHOD; }
};
```

2. **Register in factory:**

```cpp
// src/core/force_calculator.cpp
std::unique_ptr<ForceCalculator> createForceCalculator(ForceMethod method) {
    switch (method) {
        // ... existing cases
        case ForceMethod::MY_METHOD:
            return std::make_unique<MyForceCalculator>(config);
    }
}
```

3. **Add UI binding:**

```cpp
// src/main.cpp
case GLFW_KEY_4:
    system.setForceMethod(ForceMethod::MY_METHOD);
    break;
```

---

## 📈 Performance Considerations

### Critical Paths

1. **Force Calculation** - Dominates runtime
   - Direct N²: Memory bandwidth bound
   - Barnes-Hut: Tree traversal bound
   - Spatial Hash: Cell iteration bound

2. **VBO Update** - Data transfer overhead
   - SoA to interleaved conversion
   - Zero-copy minimizes overhead

### Optimization Checklist

- [ ] Use Release build (`-O3`)
- [ ] Enable CUDA fast math (`-use_fast_math`)
- [ ] Set correct GPU architecture (`-arch=sm_86`)
- [ ] Use optimal algorithm for particle count
- [ ] Tune Barnes-Hut θ parameter
- [ ] Set appropriate Spatial Hash cell size

---

## 📚 Related Documentation

- [API Reference](./api.md) - Detailed API documentation
- [Algorithms](./algorithms.md) - Algorithm explanations
- [Performance Guide](./performance.md) - Optimization strategies
- [Getting Started](./getting-started.md) - Setup and usage
