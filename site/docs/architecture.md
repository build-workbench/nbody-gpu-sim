---
title: Architecture
description: System architecture, component interactions, and design patterns of the N-Body Particle Simulation System.
---

# Architecture Overview

This document describes the system architecture, component interactions, and design patterns of the N-Body Particle Simulation System.

---

## High-Level Architecture

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

## Core Components

### 1. ParticleSystem

The central orchestrator managing all simulation components.

**Responsibilities:**
- Configuration management
- Component initialization
- Simulation main loop
- State save/load

### 2. ParticleData (SoA)

Structure of Arrays layout for GPU memory coalescing.

```cpp
struct ParticleData {
    float* position_x;
    float* position_y;
    float* position_z;
    float* velocity_x;
    float* velocity_y;
    float* velocity_z;
    float* mass;
    size_t count;
};
```

### 3. ForceCalculator (Strategy Pattern)

Abstract interface for force computation algorithms.

```cpp
class ForceCalculator {
public:
    virtual ~ForceCalculator() = default;
    virtual void computeForces(ParticleData* particles) = 0;
    virtual const char* getName() const = 0;
};
```

Implementations:
- `DirectForceCalculator` — O(N²) exact calculation
- `BarnesHutForceCalculator` — O(N log N) tree-based
- `SpatialHashForceCalculator` — O(N) grid-based

### 4. Integrator

Velocity Verlet symplectic integration.

```cpp
void velocityVerlet(
    ParticleData* particles,
    float dt,
    float softening
);
```

Properties:
- **Time-reversible**
- **Symplectic** (preserves phase space volume)
- **2nd order accurate**
- **Energy conserving** (long-term stability)

### 5. CudaGLInterop

Zero-copy bridge between CUDA and OpenGL.

```cpp
class CudaGLInterop {
public:
    void registerBuffer(GLuint vbo);
    void mapResources();
    void* getMappedPointer();
    void unmapResources();
};
```

---

## Design Patterns

### Strategy Pattern

Used for `ForceCalculator` to enable runtime algorithm switching.

```cpp
// Factory
std::unique_ptr<ForceCalculator> createForceCalculator(
    ForceMethod method,
    const SimulationConfig& config
);

// Runtime switching
system.setForceMethod(ForceMethod::BARNES_HUT);
```

### Bridge Pattern

Used for `CudaGLInterop` to separate CUDA/OpenGL implementation from application code.

### Facade Pattern

`ParticleSystem` provides a simplified interface to the complex subsystem.

---

## Memory Layout

### Structure of Arrays (SoA)

```cpp
// AoS (Array of Structures) - BAD for GPU
struct Particle { float x, y, z, vx, vy, vz, mass; };
Particle* particles;

// SoA (Structure of Arrays) - GOOD for GPU
struct ParticleData {
    float* x; float* y; float* z;
    float* vx; float* vy; float* vz;
    float* mass;
};
```

**Benefits:**
- Coalesced memory access
- Better cache utilization
- Efficient vectorization
- Reduced memory bandwidth

### Memory Footprint

| Component | Bytes per particle |
|-----------|-------------------|
| Position (x3 floats) | 12 |
| Velocity (x3 floats) | 12 |
| Acceleration (x3 floats) | 12 |
| Mass (1 float) | 4 |
| **Total** | **~52 bytes** |

---

## CUDA-OpenGL Interop

### Zero-Copy Rendering Flow

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  CUDA Kernel │ ──► │  Shared VBO  │ ──► │ OpenGL Draw │
│  (Compute)   │     │  (Zero Copy) │     │  (Render)   │
└─────────────┘     └─────────────┘     └─────────────┘
          No CPU↔GPU data transfer
```

### Implementation

```cpp
// 1. Create OpenGL buffer
GLuint vbo;
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

// 2. Register with CUDA
cudaGraphicsGLRegisterBuffer(
    &cuda_vbo_resource,
    vbo,
    cudaGraphicsMapFlagsWriteDiscard
);

// 3. Map and use
float* positions;
cudaGraphicsMapResources(1, &cuda_vbo_resource);
cudaGraphicsResourceGetMappedPointer(
    (void**)&positions,
    &size,
    cuda_vbo_resource
);
// Kernel writes directly to positions
cudaGraphicsUnmapResources(1, &cuda_vbo_resource);

// 4. Render
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glDrawArrays(GL_POINTS, 0, particle_count);
```

---

## Build System

### CMake Structure

```
├── CMakeLists.txt           # Root configuration
├── cmake/
│   ├── FindCUDA.cmake       # CUDA detection
│   └── Modules/             # Find modules
├── src/
│   ├── CMakeLists.txt       # Source build
│   └── main.cpp
└── tests/
    └── CMakeLists.txt       # Test build
```

### Key Targets

| Target | Purpose |
|--------|---------|
| `nbody_sim` | Main executable |
| `nbody_lib` | Static library |
| `nbody_tests` | Test suite |
