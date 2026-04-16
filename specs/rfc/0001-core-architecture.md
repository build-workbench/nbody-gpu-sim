# RFC 0001: Core Architecture

**Status**: Implemented  
**Version**: 2.0.0  
**Last Updated**: 2026-03-13  
**Authors**: N-Body Simulation Team

---

## Overview

This RFC describes the architecture and design of the N-Body Particle Simulation System, a high-performance GPU-accelerated simulation with real-time visualization.

### Design Goals

1. **High Performance**: Support million-particle simulations at interactive frame rates
2. **Modularity**: Clean separation between algorithms, integration, and visualization
3. **Extensibility**: Easy to add new force methods or integrators
4. **Correctness**: Verified through comprehensive testing

---

## System Architecture

### High-Level Design

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              APPLICATION LAYER                               │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                           main.cpp                                   │   │
│  │  ┌─────────────┐    ┌─────────────┐    ┌─────────────────────────┐ │   │
│  │  │   GLFW      │    │   Input     │    │   Application State     │ │   │
│  │  │   Window    │    │   Handler   │    │   Management            │ │   │
│  │  └─────────────┘    └─────────────┘    └─────────────────────────┘ │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                              SIMULATION LAYER                                │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                        ParticleSystem                                │   │
│  │  ┌───────────────┐  ┌───────────────┐  ┌───────────────────────┐   │   │
│  │  │ ForceCalculator│  │   Integrator  │  │   CudaGLInterop       │   │   │
│  │  │   (Strategy)   │  │ (Velocity     │  │   (Zero-Copy          │   │   │
│  │  │               │  │  Verlet)       │  │    Bridge)            │   │   │
│  │  └───────┬───────┘  └───────────────┘  └───────────────────────┘   │   │
│  │          │                                                         │   │
│  │  ┌───────▼────────────────────────────────────────────────────┐    │   │
│  │  │              Force Calculation Methods                      │    │   │
│  │  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐   │    │   │
│  │  │  │ Direct N²   │ │ Barnes-Hut  │ │   Spatial Hash      │   │    │   │
│  │  │  │ O(N²)       │ │ O(N log N)  │ │   O(N) short-range  │   │    │   │
│  │  │  └─────────────┘ └─────────────┘ └─────────────────────┘   │    │   │
│  │  └────────────────────────────────────────────────────────────┘    │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                              RENDERING LAYER                                 │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                          Renderer                                    │   │
│  │  ┌───────────────┐  ┌───────────────┐  ┌───────────────────────┐   │   │
│  │  │   Camera      │  │   Shaders     │  │   Color Mapping       │   │   │
│  │  │   Controller  │  │   (GLSL)      │  │   (Velocity/Depth)    │   │   │
│  │  └───────────────┘  └───────────────┘  └───────────────────────┘   │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                              GPU MEMORY LAYER                                │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    ParticleData (SoA Layout)                         │   │
│  │  pos_x[N] pos_y[N] pos_z[N] | vel_x[N] vel_y[N] vel_z[N] | ...     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Core Components

### 1. ParticleSystem

The central orchestrator managing all simulation components.

**Responsibilities**:
- Coordinate initialization, updates, and cleanup
- Manage particle data lifecycle
- Provide state serialization

**Key Methods**:
```cpp
void initialize(const SimulationConfig& config);
void update(float dt);
void setForceMethod(ForceMethod method);
void saveState(const std::string& filename);
```

---

### 2. ForceCalculator (Strategy Pattern)

Abstract interface enabling algorithm switching at runtime.

**Hierarchy**:
```
ForceCalculator (abstract)
├── DirectForceCalculator
├── BarnesHutCalculator
└── SpatialHashCalculator
```

**Design Rationale**:
- Strategy pattern allows runtime algorithm selection
- Common interface simplifies testing and integration
- Each implementation can have algorithm-specific parameters

---

### 3. Integrator

Implements Velocity Verlet symplectic integration.

**Algorithm**:
```
1. Store old accelerations: a_old = a(t)
2. Update positions: x(t+dt) = x(t) + v(t)·dt + ½·a(t)·dt²
3. Compute new forces: F(x(t+dt)) → a(t+dt)
4. Update velocities: v(t+dt) = v(t) + ½·(a_old + a(t+dt))·dt
```

**Properties**:
- Symplectic: Preserves phase space volume
- Second-order accurate
- Single force evaluation per step

---

### 4. CudaGLInterop (Bridge Pattern)

Enables zero-copy sharing between CUDA and OpenGL.

**Workflow**:
```
CUDA Compute → Map VBO → Copy Positions → Unmap VBO → OpenGL Render
```

**Benefits**:
- No CPU-GPU data transfer
- Minimal latency
- Direct rendering from simulation data

---

## Data Structures

### ParticleData (SoA Layout)

```cpp
struct ParticleData {
    // Position (3N floats)
    float* pos_x, *pos_y, *pos_z;

    // Velocity (3N floats)
    float* vel_x, *vel_y, *vel_z;

    // Acceleration (6N floats - current + old)
    float* acc_x, *acc_y, *acc_z;
    float* acc_old_x, *acc_old_y, *acc_old_z;

    // Mass (N floats)
    float* mass;

    size_t count;
};
```

**Memory per particle**: 13 floats = 52 bytes

**Rationale**:
- SoA enables memory coalescing on GPU
- Better cache utilization for sequential access patterns
- Compatible with CUDA vectorized memory operations

---

## Algorithm Details

### Direct N² Force Calculation

**Complexity**: O(N²)

**Optimizations**:
1. Shared memory tiling for particle data caching
2. Hardware rsqrt for inverse square root
3. Coalesced memory access via SoA layout

**Pseudocode**:
```cuda
for each tile of particles:
    load tile into shared memory
    __syncthreads()
    for each particle j in tile:
        compute force contribution
    __syncthreads()
```

---

### Barnes-Hut Algorithm

**Complexity**: O(N log N)

**Components**:
1. Morton code computation for spatial sorting
2. Octree construction
3. Center of mass propagation
4. Tree traversal for force calculation

**θ Parameter**:
- Controls approximation accuracy
- θ = 0: Exact (equivalent to Direct N²)
- θ = 0.5: Default, good balance
- θ = 1.0: Fast but less accurate

---

### Spatial Hash Algorithm

**Complexity**: O(N) for short-range forces

**Components**:
1. Cell assignment for all particles
2. Particle sorting by cell index
3. Cell range computation (prefix sum)
4. Neighbor search within 3×3×3 cell neighborhood

**Cutoff Radius**:
- Forces computed only for pairs within cutoff
- Ideal for molecular dynamics and SPH

---

## Correctness Properties

The system is validated against the following properties:

### Property 1: Force Calculation Correctness
- Force magnitude approximates G·m₁·m₂/(r² + ε²)
- Force direction points from source to target
- Force remains finite even at zero distance (softening)

### Property 2: Barnes-Hut Tree Structure
- Tree contains exactly N particles
- Mass is conserved across the tree
- Center of mass computed correctly

### Property 3: Barnes-Hut Approximation Convergence
- Lower θ → more accurate results
- θ → 0 converges to Direct N²

### Property 4: Force Method Equivalence
- Barnes-Hut (low θ) ≈ Direct N²
- Spatial Hash matches Direct N² within cutoff

### Property 5: Spatial Hash Cell Assignment
- Each particle assigned to exactly one cell
- Cell bounds contain particle position

### Property 6: Spatial Hash Neighbor Cutoff
- All particles within cutoff are included
- All particles beyond cutoff are excluded

### Property 7: Energy Conservation
- Total energy oscillates but doesn't drift
- Symplectic integrator preserves phase space

### Property 8: CUDA-GL Interop Data Integrity
- Data round-trip preserves values
- No corruption during map/unmap cycles

### Property 9: Camera Transformation
- View matrix correctly transforms coordinates
- Projection preserves relative positions

### Property 10: Color Mapping
- Colors are in valid RGB range [0,1]
- Mapping is monotonic

### Property 11: Pause/Resume State Preservation
- State unchanged during pause
- Simulation continues correctly after resume

### Property 12: Save/Load Round-Trip
- Serialization preserves all state
- Loaded state equals original

### Property 13: Input Validation
- Invalid inputs are rejected
- System state unchanged after rejection

### Property 14: Particle Distribution Bounds
- All particles within specified bounds
- Distribution respects parameters

---

## Extension Points

### Adding New Force Methods

1. Create class inheriting from `ForceCalculator`
2. Implement `computeForces(ParticleData*)`
3. Add enum value to `ForceMethod`
4. Register in `createForceCalculator()` factory

### Adding New Distributions

1. Create parameter struct
2. Implement `ParticleInitializer::initNewDistribution()`
3. Add enum value to `InitDistribution`
4. Add case in `ParticleSystem::initialize()`

### Adding New Integrators

1. Create class with `integrate()` method
2. Implement position/velocity update kernels
3. Add factory method if multiple options needed

---

## Performance Considerations

### GPU Memory Bandwidth
- SoA layout enables coalesced access
- Shared memory reduces global memory traffic
- Persistent buffers avoid allocation overhead

### Kernel Optimization
- Thread block size tuned for architecture
- Warp divergence minimized
- Register pressure managed

### Zero-Copy Rendering
- VBO shared between CUDA and OpenGL
- No CPU staging buffers
- Minimal synchronization

---

## Testing Strategy

### Unit Tests
- Individual component verification
- Edge case coverage
- Mock interfaces where needed

### Property-Based Tests (RapidCheck)
- Automatic test case generation
- Universal property verification
- Regression prevention

### Integration Tests
- Component interaction verification
- Full pipeline testing
- Performance benchmarks

---

## References

1. Barnes, J., & Hut, P. (1986). A hierarchical O(N log N) force-calculation algorithm. *Nature*, 324(6096), 446-449.

2. Nyland, L., Harris, M., & Prins, J. (2007). Fast N-body simulation with CUDA. *GPU Gems 3*, 677-695.

3. Verlet, L. (1967). Computer "experiments" on classical fluids. *Physical Review*, 159(1), 98.
