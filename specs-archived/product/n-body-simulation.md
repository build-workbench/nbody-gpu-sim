# Product Specification: N-Body Particle Simulation System

**Version**: 2.0.0  
**Status**: Implemented  
**Last Updated**: 2026-03-13

---

## Overview

A high-performance GPU-accelerated N-Body particle simulation system capable of simulating millions of particles with real-time visualization. The system uses CUDA for parallel computation and CUDA-OpenGL interop for zero-copy rendering.

### Key Features

- **Multiple Force Algorithms**: Direct N², Barnes-Hut (O(N log N)), Spatial Hash (O(N))
- **Symplectic Integration**: Velocity Verlet for energy conservation
- **Real-time Visualization**: Zero-copy CUDA-OpenGL interop
- **Flexible Initialization**: Uniform, spherical, and disk distributions

---

## Glossary

| Term | Definition |
|------|------------|
| **ParticleSystem** | Core class managing particle data and simulation state |
| **ForceCalculator** | Abstract interface for force computation algorithms |
| **BarnesHutTree** | Octree data structure for O(N log N) force approximation |
| **SpatialHashGrid** | Uniform grid for O(N) short-range force computation |
| **Integrator** | Time integration module using Velocity Verlet scheme |
| **CudaGLInterop** | Bridge for zero-copy data sharing between CUDA and OpenGL |
| **Renderer** | OpenGL-based particle visualization system |

---

## Functional Requirements

### FR-1: Particle Data Management

**Priority**: Critical  
**User Story**: As a simulation developer, I want to efficiently manage particle data for large-scale simulations.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-1.1 | System SHALL use Structure of Arrays (SoA) layout for GPU memory coalescing | Critical |
| FR-1.2 | System SHALL support at least 10 million particles | High |
| FR-1.3 | System SHALL provide configurable initial distributions (uniform, spherical, disk) | Medium |
| FR-1.4 | System SHALL store position, velocity, acceleration, and mass for each particle | Critical |

---

### FR-2: Force Calculation

**Priority**: Critical  
**User Story**: As a physics simulation user, I want accurate and efficient force calculations.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-2.1 | System SHALL compute gravitational forces using Newton's law | Critical |
| FR-2.2 | System SHALL use GPU parallel computation with one thread per particle | Critical |
| FR-2.3 | System SHALL use CUDA Shared Memory for data caching | High |
| FR-2.4 | System SHALL use hardware rsqrt for inverse square root | Medium |
| FR-2.5 | System SHALL apply softening parameter to prevent numerical singularities | Critical |

---

### FR-3: Barnes-Hut Algorithm

**Priority**: High  
**User Story**: As a performance-conscious user, I want O(N log N) algorithm for large simulations.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-3.1 | System SHALL construct octree from particle positions | Critical |
| FR-3.2 | System SHALL compute center of mass and total mass per node | Critical |
| FR-3.3 | System SHALL use configurable θ parameter for approximation | High |
| FR-3.4 | System SHALL rebuild tree each simulation step | Critical |
| FR-3.5 | System SHALL support runtime switching between algorithms | Medium |

---

### FR-4: Spatial Hash Algorithm

**Priority**: Medium  
**User Story**: As a molecular dynamics user, I want O(N) short-range force computation.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-4.1 | System SHALL partition 3D space into uniform grid cells | Critical |
| FR-4.2 | System SHALL assign particles to cells efficiently | Critical |
| FR-4.3 | System SHALL only process particles in neighboring cells | Critical |
| FR-4.4 | System SHALL support configurable cell size and cutoff radius | Medium |

---

### FR-5: Numerical Integration

**Priority**: Critical  
**User Story**: As a simulation user, I want stable and accurate time integration.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-5.1 | System SHALL implement Velocity Verlet integration | Critical |
| FR-5.2 | System SHALL support configurable time step | High |
| FR-5.3 | System SHALL update all particles in parallel on GPU | Critical |
| FR-5.4 | System SHALL provide energy conservation metrics | Medium |

---

### FR-6: CUDA-OpenGL Interoperability

**Priority**: Critical  
**User Story**: As a visualization user, I want zero-copy rendering for real-time display.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-6.1 | System SHALL register OpenGL buffers with CUDA | Critical |
| FR-6.2 | System SHALL map particle positions directly to vertex buffers | Critical |
| FR-6.3 | System SHALL avoid CPU-GPU data transfers during rendering | Critical |
| FR-6.4 | System SHALL support dynamic buffer resizing | Low |

---

### FR-7: Real-time Visualization

**Priority**: High  
**User Story**: As a user, I want to observe simulation behavior in real-time.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-7.1 | System SHALL display particles as point sprites | Critical |
| FR-7.2 | System SHALL support camera orbit and zoom controls | High |
| FR-7.3 | System SHALL display simulation statistics | Medium |
| FR-7.4 | System SHALL support multiple color modes (depth, velocity, density) | Medium |
| FR-7.5 | System SHALL maintain 30+ FPS for 1M particles | High |

---

### FR-8: Simulation Control

**Priority**: High  
**User Story**: As a user, I want runtime control over simulation parameters.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-8.1 | System SHALL support pause, resume, and reset | Critical |
| FR-8.2 | System SHALL allow runtime force method switching | High |
| FR-8.3 | System SHALL support saving and loading state | Medium |
| FR-8.4 | System SHALL allow parameter adjustment at runtime | Medium |

---

### FR-9: Error Handling

**Priority**: High  
**User Story**: As a user, I want graceful error handling and recovery.

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-9.1 | System SHALL report detailed CUDA initialization errors | Critical |
| FR-9.2 | System SHALL validate all user inputs | Critical |
| FR-9.3 | System SHALL check GPU memory availability before allocation | High |
| FR-9.4 | System SHALL provide meaningful error messages | High |

---

## Non-Functional Requirements

### NFR-1: Performance

| ID | Requirement | Target |
|----|-------------|--------|
| NFR-1.1 | Frame rate for 100K particles | 60+ FPS |
| NFR-1.2 | Frame rate for 1M particles (Barnes-Hut) | 25+ FPS |
| NFR-1.3 | Frame rate for 1M particles (Spatial Hash) | 60+ FPS |
| NFR-1.4 | Memory efficiency | <200 MB for 1M particles |

### NFR-2: Reliability

| ID | Requirement | Target |
|----|-------------|--------|
| NFR-2.1 | Energy drift per 1000 steps | <1% |
| NFR-2.2 | Test coverage | >90% |
| NFR-2.3 | Property-based test validation | All properties |

### NFR-3: Usability

| ID | Requirement | Target |
|----|-------------|--------|
| NFR-3.1 | Build complexity | Single cmake command |
| NFR-3.2 | Documentation completeness | All APIs documented |
| NFR-3.3 | Example availability | 4+ examples |

---

## Acceptance Criteria

- [ ] All functional requirements implemented and tested
- [ ] Performance targets met on reference hardware (RTX 3080)
- [ ] Test coverage >90% for core components
- [ ] All property-based tests passing
- [ ] Documentation complete and reviewed

---

## Implementation Status

**Version 2.0.0**: All 39 tasks across 11 phases complete.

| Phase | Tasks | Status |
|-------|-------|--------|
| 1. Project Setup | 1 | ✅ Complete |
| 2. Particle Data | 3 | ✅ Complete |
| 3. Direct N² Force | 3 | ✅ Complete |
| 4. Numerical Integration | 2 | ✅ Complete |
| 5. Barnes-Hut | 6 | ✅ Complete |
| 6. Spatial Hash | 6 | ✅ Complete |
| 7. CUDA-GL Interop | 2 | ✅ Complete |
| 8. Rendering System | 5 | ✅ Complete |
| 9. Particle System | 5 | ✅ Complete |
| 10. Error Handling | 4 | ✅ Complete |
| 11. Main Application | 2 | ✅ Complete |

---

## Traceability Matrix

| Requirement | Test File | Implementation |
|-------------|-----------|----------------|
| FR-1.x | `test_particle_data.cpp` | `particle_data.hpp`, `particle_init.cu` |
| FR-2.x | `test_force_calculation.cpp` | `force_calculator.hpp`, `force_direct.cu` |
| FR-3.x | `test_barnes_hut.cpp` | `barnes_hut_tree.hpp`, `force_barnes_hut.cu` |
| FR-4.x | `test_spatial_hash.cpp` | `spatial_hash_grid.hpp`, `force_spatial_hash.cu` |
| FR-5.x | `test_integrator.cpp` | `integrator.hpp`, `integrator.cu` |
| FR-6.x | `test_force_calculation.cpp` | `cuda_gl_interop.hpp` |
| FR-7.x | `test_camera.cpp`, `test_color_mapping.cpp` | `renderer.hpp`, `camera.hpp` |
| FR-8.x | `test_serialization.cpp`, `test_validation.cpp` | `particle_system.hpp`, `serialization.hpp` |
| FR-9.x | `test_validation.cpp` | `error_handling.hpp` |
