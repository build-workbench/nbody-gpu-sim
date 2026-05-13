# Architecture

System architecture and design patterns.

## System Overview

```mermaid
graph TB
    subgraph CPU["CPU (Host)"]
        App[Application<br/>CLI / Example]
        PS[ParticleSystem<br/>Facade]
        Config[SimulationConfig]
        Ser[Serialization<br/>Save / Load]
        HDF5[HDF5 I/O]
    end
    
    subgraph GPU["GPU (Device)"]
        FK[Force Calculator<br/>Strategy Pattern]
        INT[Integrator<br/>Velocity Verlet]
        REN[Renderer<br/>Point Sprites]
        GL[OpenGL Interop<br/>Zero-Copy]
    end
    
    App --> PS
    Config --> PS
    PS --> FK
    PS --> INT
    PS --> REN
    FK --> INT
    INT --> REN
    REN --> GL
    PS --> Ser
    PS --> HDF5
```

## Design Patterns

### Strategy Pattern (Force Calculation)

Runtime algorithm switching through a common interface:

```mermaid
classDiagram
    class ParticleSystem {
        -ForceCalculator* calculator_
        +initialize(config)
        +update(dt)
        +setForceMethod(method)
    }
    
    class ForceCalculator {
        <<interface>>
        +compute(data)
    }
    
    class DirectForce {
        +compute(data)
    }
    
    class BarnesHutForce {
        +compute(data)
    }
    
    class SpatialHashForce {
        +compute(data)
    }
    
    ParticleSystem --> ForceCalculator
    ForceCalculator <|.. DirectForce
    ForceCalculator <|.. BarnesHutForce
    ForceCalculator <|.. SpatialHashForce
```

### Bridge Pattern (CUDA-OpenGL)

Decouples CUDA computation from OpenGL rendering:

```mermaid
graph LR
    A[CUDA Buffer] --> B[CudaGLInterop]
    B --> C[OpenGL VBO]
    C --> D[Renderer]
    
    style B fill:#10b981,stroke:#059669,color:#fff
```

### Facade Pattern (ParticleSystem)

Simplifies the complex subsystem into a single API:

```cpp
// Without facade
ForceCalculator* calc = ForceCalculator::create(method);
Integrator integrator;
Renderer renderer;
CudaGLInterop interop;
// ... complex wiring ...

// With facade
ParticleSystem system;
system.initialize(config);
system.update(dt);
```

## Memory Architecture

### Structure of Arrays (SoA)

Particle data is stored in SoA layout for GPU memory coalescing:

```mermaid
graph LR
    subgraph SoA["ParticleData (SoA)"]
        PX["position_x[N]"]
        PY["position_y[N]"]
        PZ["position_z[N]"]
        VX["velocity_x[N]"]
        VY["velocity_y[N]"]
        VZ["velocity_z[N]"]
        FX["force_x[N]"]
        FY["force_y[N]"]
        FZ["force_z[N]"]
        M["mass[N]"]
    end
    
    GPU["GPU Global Memory"]
    
    PX --> GPU
    PY --> GPU
    PZ --> GPU
```

### Zero-Copy Rendering

CUDA-OpenGL interop shares memory between compute and rendering:

```mermaid
sequenceDiagram
    participant K as CUDA Kernel
    participant M as GPU Memory
    participant O as OpenGL
    
    K->>M: Write positions
    M->>O: Zero-copy VBO
    O->>O: Point sprite render
    Note over M,O: No CPU transfer needed
```

## Simulation Loop

```mermaid
sequenceDiagram
    participant App as Application
    participant PS as ParticleSystem
    participant FC as ForceCalculator
    participant I as Integrator
    participant R as Renderer
    
    loop Each Frame
        App->>PS: update(dt)
        PS->>FC: computeForces()
        FC-->>PS: forces computed
        PS->>I: integrate(dt)
        I-->>PS: positions updated
        PS->>R: render()
        R-->>PS: frame rendered
    end
```

## Component Responsibilities

| Component | Responsibility |
|-----------|---------------|
| `ParticleSystem` | Orchestration, state management |
| `ForceCalculator` | Force computation (strategy) |
| `Integrator` | Time integration (Velocity Verlet) |
| `Renderer` | OpenGL visualization |
| `CudaGLInterop` | GPU memory sharing |
| `Camera` | View control |
| `UIPanel` | Dear ImGui diagnostics |

## Extension Points

### Adding a New Force Method

1. Create a class implementing `ForceCalculator`
2. Add to the `ForceMethod` enum
3. Register in `ForceCalculator::create()`

```cpp
class MyCustomForce : public ForceCalculator {
public:
    void compute(ParticleData& data) override {
        // Custom force computation
    }
};
```

### Adding a New Renderer

1. Create a class implementing the rendering interface
2. Integrate with `CudaGLInterop` for zero-copy
3. Register in the rendering factory

## Source Layout

| Path | Purpose |
|------|---------|
| `src/core/` | ParticleSystem, CLI, algorithm stubs |
| `src/cuda/` | CUDA kernels (force, integration, init) |
| `src/render/` | OpenGL rendering, camera, interop |
| `src/utils/` | Serialization, HDF5, profiling |
| `include/nbody/` | Public headers |
