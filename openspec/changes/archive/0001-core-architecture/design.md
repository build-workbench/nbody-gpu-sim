# Design: Core Architecture

## Context

The N-body particle simulation system requires a high-performance, maintainable architecture that:
- Supports GPU acceleration via CUDA
- Enables real-time visualization with OpenGL
- Provides flexible algorithm selection at runtime
- Maintains code quality through clear separation of concerns

This architecture was designed at project inception to establish foundational patterns.

---

## Goals

1. **High Performance**: Support million-particle simulations at interactive frame rates
2. **Modularity**: Clean separation between algorithms, integration, and visualization
3. **Extensibility**: Easy to add new force methods or integrators
4. **Correctness**: Verified through comprehensive testing

---

## Non-Goals

1. Distributed/multi-GPU support
2. Real-time network streaming
3. VR/AR rendering
4. Plugin system for third-party extensions

---

## Decisions

### D1: Four-Layer Architecture

**Decision:** Implement distinct layers with clear responsibilities.

```
┌─────────────────────────────────────────────────────────────┐
│                     APPLICATION LAYER                        │
│  GLFW Window │ InputHandler │ Application State Management  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     SIMULATION LAYER                         │
│  ParticleSystem (Facade)                                     │
│  ├── ForceCalculator (Strategy)                              │
│  ├── Integrator (Velocity Verlet)                            │
│  └── CudaGLInterop (Bridge)                                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     RENDERING LAYER                          │
│  Renderer │ Camera │ Shaders │ ColorMapping                 │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    GPU MEMORY LAYER                          │
│  ParticleData (SoA Layout)                                   │
│  pos_x[N] pos_y[N] pos_z[N] │ vel_x[N] vel_y[N] vel_z[N]    │
└─────────────────────────────────────────────────────────────┘
```

**Rationale:**
- Clear separation enables independent testing
- Each layer has well-defined interfaces
- Changes in one layer don't ripple to others

---

### D2: Strategy Pattern for Force Calculators

**Decision:** Use Strategy pattern for runtime algorithm selection.

```cpp
class ForceCalculator {
public:
    virtual void computeForces(ParticleData* data) = 0;
    virtual ~ForceCalculator() = default;
};

class DirectForceCalculator : public ForceCalculator { ... };
class BarnesHutCalculator : public ForceCalculator { ... };
class SpatialHashCalculator : public ForceCalculator { ... };
```

**Rationale:**
- Users can switch algorithms without code changes
- Easy to benchmark different methods
- New algorithms can be added without modifying existing code

---

### D3: Structure of Arrays (SoA) Layout

**Decision:** Store particle attributes in separate contiguous arrays.

```cpp
struct ParticleData {
    float* pos_x, *pos_y, *pos_z;  // Position
    float* vel_x, *vel_y, *vel_z;  // Velocity
    float* acc_x, *acc_y, *acc_z;  // Acceleration
    float* mass;                    // Mass
    size_t count;
};
```

**Rationale:**
- Enables memory coalescing on GPU
- Better cache utilization for sequential access
- Compatible with CUDA vectorized memory operations

---

### D4: Velocity Verlet Integration

**Decision:** Use symplectic Velocity Verlet integrator.

**Rationale:**
- Preserves phase space volume (symplectic)
- Second-order accurate
- Single force evaluation per step
- Better energy conservation than Euler methods

---

### D5: Zero-Copy CUDA-GL Interop

**Decision:** Share GPU memory between CUDA and OpenGL.

```cpp
// Register VBO with CUDA
cudaGraphicsGLRegisterBuffer(&resource, vbo, cudaGraphicsMapFlagsNone);

// Map for CUDA access
cudaGraphicsMapResources(1, &resource);
cudaGraphicsResourceGetMappedPointer(&devPtr, &size, resource);

// Direct GPU access - no CPU copy
computeKernel<<<...>>>(devPtr, ...);

// Unmap for OpenGL rendering
cudaGraphicsUnmapResources(1, &resource);
```

**Rationale:**
- Eliminates CPU-GPU data transfer latency
- Essential for real-time performance
- Minimal synchronization overhead

---

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| CUDA-GL interop complexity | Encapsulate in CudaGLInterop class with clear interface |
| SoA code verbosity | Provide utility macros and inline functions |
| Strategy pattern overhead | Virtual call overhead is negligible compared to GPU kernel time |
| Verlet integration memory | Store old accelerations - acceptable for energy conservation benefit |

---

## Alternative Approaches Considered

### Alternative 1: Array of Structures (AoS)

```cpp
struct Particle {
    float pos[3], vel[3], acc[3], mass;
};
Particle particles[N];
```

**Rejected because:** Poor GPU memory coalescing, scattered memory access patterns.

### Alternative 2: Euler Integration

**Rejected because:** Energy drift over time, not symplectic, less accurate.

### Alternative 3: Multi-pass Rendering

**Rejected because:** Unnecessary complexity for particle rendering, single-pass is sufficient.

---

## References

1. Barnes, J., & Hut, P. (1986). A hierarchical O(N log N) force-calculation algorithm. *Nature*, 324(6096), 446-449.

2. Nyland, L., Harris, M., & Prins, J. (2007). Fast N-body simulation with CUDA. *GPU Gems 3*, 677-695.

3. Verlet, L. (1967). Computer "experiments" on classical fluids. *Physical Review*, 159(1), 98.
