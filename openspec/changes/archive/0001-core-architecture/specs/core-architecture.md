# Capability: Core Architecture

## Overview

Defines the foundational system architecture with four distinct layers: GPU Memory, Simulation, Rendering, and Application.

---

## ADDED Requirements

### Requirement: Layered Architecture

The system SHALL implement a four-layer architecture for separation of concerns.

#### Scenario: GPU Memory Layer
- **WHEN** particle data is stored
- **THEN** the system SHALL use SoA layout in GPU memory

#### Scenario: Simulation Layer
- **WHEN** physics computation is performed
- **THEN** ParticleSystem SHALL orchestrate ForceCalculator, Integrator, and CudaGLInterop components

#### Scenario: Rendering Layer
- **WHEN** visualization is required
- **THEN** Renderer SHALL display particles via OpenGL with Camera and ColorMapping components

#### Scenario: Application Layer
- **WHEN** the program runs
- **THEN** main.cpp SHALL manage GLFW window, InputHandler, and application state

---

### Requirement: Strategy Pattern for Force Calculation

Force calculators SHALL implement the Strategy pattern for runtime algorithm switching.

#### Scenario: Interface Definition
- **WHEN** a force calculator is implemented
- **THEN** it SHALL inherit from ForceCalculator abstract base class

#### Scenario: Runtime Selection
- **WHEN** user selects a force method
- **THEN** the system SHALL switch algorithms without code changes

#### Scenario: Available Implementations
- **WHEN** force calculation is needed
- **THEN** DirectForceCalculator, BarnesHutCalculator, and SpatialHashCalculator SHALL be available

---

### Requirement: Bridge Pattern for CUDA-GL Interop

CUDA-OpenGL interoperability SHALL use the Bridge pattern.

#### Scenario: Zero-Copy Data Sharing
- **WHEN** particle positions are updated by CUDA
- **THEN** OpenGL vertex buffers SHALL reference the same GPU memory

#### Scenario: Map/Unmap Cycle
- **WHEN** rendering frame
- **THEN** buffers SHALL be mapped to CUDA, updated, then unmapped for OpenGL

---

### Requirement: Facade Pattern for ParticleSystem

ParticleSystem SHALL implement the Facade pattern for simplified API.

#### Scenario: Unified Interface
- **WHEN** external code interacts with simulation
- **THEN** ParticleSystem SHALL provide a simplified interface hiding internal complexity

#### Scenario: Lifecycle Management
- **WHEN** simulation is controlled
- **THEN** initialize(), update(), setForceMethod(), saveState() SHALL be available

---

## Design Decisions

### SoA Layout

Memory layout uses Structure of Arrays for GPU coalescing:

```
ParticleData {
    float* pos_x, pos_y, pos_z;     // 3N floats
    float* vel_x, vel_y, vel_z;     // 3N floats
    float* acc_x, acc_y, acc_z;     // 3N floats
    float* acc_old_x, acc_old_y, acc_old_z; // 3N floats
    float* mass;                     // N floats
}
// Total: 13N floats = 52N bytes per particle
```

### Velocity Verlet Integration

Symplectic integrator for energy conservation:

1. Store old accelerations: a_old = a(t)
2. Update positions: x(t+dt) = x(t) + v(t)·dt + ½·a(t)·dt²
3. Compute new forces: F(x(t+dt)) → a(t+dt)
4. Update velocities: v(t+dt) = v(t) + ½·(a_old + a(t+dt))·dt

### Correctness Properties

14 correctness properties defined for validation:
1. Force calculation correctness (magnitude, direction, softening)
2. Barnes-Hut tree structure (N particles, mass conservation)
3. Barnes-Hut approximation convergence
4. Force method equivalence
5. Spatial hash cell assignment
6. Spatial hash neighbor cutoff
7. Energy conservation
8. CUDA-GL interop data integrity
9. Camera transformation
10. Color mapping validity
11. Pause/resume state preservation
12. Save/load round-trip
13. Input validation
14. Particle distribution bounds

---

## Traceability

| Component | Header File | Implementation |
|-----------|-------------|----------------|
| ParticleData | particle_data.hpp | particle_init.cu |
| ParticleSystem | particle_system.hpp | particle_system.cpp |
| ForceCalculator | force_calculator.hpp | force_*.cu |
| Integrator | integrator.hpp | integrator.cu |
| CudaGLInterop | cuda_gl_interop.hpp | cuda_gl_interop.cpp |
| Renderer | renderer.hpp | renderer.cpp |
| Camera | camera.hpp | camera.cpp |
