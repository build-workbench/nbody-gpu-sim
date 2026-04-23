# Proposal: Core Architecture

## Why

The N-body simulation requires a scalable, maintainable architecture that separates concerns between physics computation, rendering, and user interaction while enabling GPU acceleration via CUDA.

This foundational architecture was designed to:
- Support million-particle simulations at interactive frame rates
- Provide clean modularity between algorithms, integration, and visualization
- Enable easy extensibility for new force methods or integrators
- Ensure correctness through comprehensive testing

## What Changes

This foundational change establishes the core architectural layers:

- **GPU Memory Layer**: Structure of Arrays (SoA) particle data layout for memory coalescing
- **Simulation Layer**: ParticleSystem orchestrator with Strategy pattern for force calculators
- **Rendering Layer**: OpenGL 3.3+ visualization with zero-copy CUDA-GL interop
- **Application Layer**: GLFW window management and input handling

## Capabilities

### New Capabilities

| Capability | Description |
|------------|-------------|
| `core-architecture` | Layered system architecture with defined interfaces |

### Modified Capabilities

None (initial architecture)

## Impact

- **Performance**: Enables GPU acceleration path via CUDA
- **Maintainability**: Clear separation of concerns across layers
- **Testability**: Each layer can be tested independently
- **Extensibility**: Strategy pattern allows runtime algorithm selection
