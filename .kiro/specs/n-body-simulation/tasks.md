# Implementation Plan: N-Body Particle Simulation System

## Overview

本实现计划将设计文档分解为可执行的编码任务。采用增量开发方式，从核心数据结构开始，逐步构建力计算、积分、加速结构和渲染模块。

## Tasks

- [x] 1. Set up project structure and build system
  - Create CMake configuration with CUDA and OpenGL support
  - Set up directory structure: `src/`, `include/`, `tests/`, `shaders/`
  - Configure Google Test + RapidCheck for property-based testing
  - _Requirements: 9.1, 9.3_

- [x] 2. Implement Particle Data Management
  - [x] 2.1 Create ParticleData SoA structure and memory management
    - Implement `ParticleData` struct with SoA layout
    - Implement CUDA memory allocation/deallocation
    - Implement host-device data transfer utilities
    - _Requirements: 1.1, 1.2, 1.4_

  - [x] 2.2 Implement particle initialization distributions
    - Implement uniform distribution initializer
    - Implement spherical distribution initializer
    - Implement disk distribution initializer
    - _Requirements: 1.3_

  - [x] 2.3 Write property test for particle distribution bounds
    - **Property 14: Particle Distribution Bounds**
    - **Validates: Requirements 1.3**

- [x] 3. Implement Direct N² Force Calculation
  - [x] 3.1 Implement basic force calculation kernel
    - Create `computeForcesDirectKernel` with shared memory tiling
    - Implement softening parameter handling
    - Use hardware rsqrt instruction
    - _Requirements: 2.1, 2.2, 2.4, 2.5_

  - [x] 3.2 Implement ForceCalculator interface and DirectForceCalculator
    - Create abstract `ForceCalculator` base class
    - Implement `DirectForceCalculator` class
    - Add configurable block size
    - _Requirements: 2.2, 2.3_

  - [x] 3.3 Write property test for force calculation correctness
    - **Property 1: Force Calculation Correctness**
    - **Validates: Requirements 2.1, 2.4, 2.5**

- [x] 4. Checkpoint - Verify core force calculation
  - Ensure all tests pass, ask the user if questions arise.

- [x] 5. Implement Velocity Verlet Integrator
  - [x] 5.1 Create Integrator class with position/velocity update kernels
    - Implement `updatePositionsKernel`
    - Implement `updateVelocitiesKernel`
    - Implement `Integrator` class orchestrating both steps
    - _Requirements: 5.1, 5.2, 5.3_

  - [x] 5.2 Write property test for energy conservation
    - **Property 7: Energy Conservation (Symplectic Integration)**
    - **Validates: Requirements 5.1, 5.4**

- [x] 6. Implement Barnes-Hut Tree Acceleration
  - [x] 6.1 Implement OctreeNode structure and tree construction
    - Create `OctreeNode` struct
    - Implement Morton code computation
    - Implement GPU tree construction
    - _Requirements: 3.1, 3.4_

  - [x] 6.2 Implement center of mass computation
    - Implement bottom-up mass aggregation kernel
    - Compute center of mass for internal nodes
    - _Requirements: 3.2_

  - [x] 6.3 Implement Barnes-Hut force calculation kernel
    - Create `barnesHutForceKernel` with theta parameter
    - Implement tree traversal on GPU
    - _Requirements: 3.3_

  - [x] 6.4 Implement BarnesHutCalculator class
    - Create `BarnesHutCalculator` inheriting from `ForceCalculator`
    - Integrate tree building and force calculation
    - _Requirements: 3.5_

  - [x] 6.5 Write property test for tree structure correctness
    - **Property 2: Barnes-Hut Tree Structure Correctness**
    - **Validates: Requirements 3.1, 3.2**

  - [x] 6.6 Write property test for approximation convergence
    - **Property 3: Barnes-Hut Approximation Convergence**
    - **Validates: Requirements 3.3**

- [x] 7. Checkpoint - Verify Barnes-Hut implementation
  - Ensure all tests pass, ask the user if questions arise.

- [x] 8. Implement Spatial Hash Grid Acceleration
  - [x] 8.1 Implement SpatialHashGrid structure
    - Create grid data structures (cell_start, cell_end, sorted_indices)
    - Implement cell assignment kernel
    - Implement particle sorting by cell
    - _Requirements: 4.1, 4.2_

  - [x] 8.2 Implement spatial hash force calculation kernel
    - Create `spatialHashForceKernel` with neighbor cell iteration
    - Implement cutoff radius handling
    - _Requirements: 4.3, 4.4_

  - [x] 8.3 Implement SpatialHashCalculator class
    - Create `SpatialHashCalculator` inheriting from `ForceCalculator`
    - Integrate grid building and force calculation
    - _Requirements: 4.3_

  - [x] 8.4 Write property test for cell assignment correctness
    - **Property 5: Spatial Hash Cell Assignment Correctness**
    - **Validates: Requirements 4.1, 4.2**

  - [x] 8.5 Write property test for neighbor cutoff
    - **Property 6: Spatial Hash Neighbor Cutoff**
    - **Validates: Requirements 4.3**

  - [x] 8.6 Write property test for force method equivalence
    - **Property 4: Force Method Equivalence**
    - **Validates: Requirements 3.5**

- [x] 9. Checkpoint - Verify all force calculation methods
  - Ensure all tests pass, ask the user if questions arise.

- [x] 10. Implement CUDA-OpenGL Interoperability
  - [x] 10.1 Implement CudaGLInterop class
    - Create OpenGL VBO for positions
    - Register VBO with CUDA
    - Implement map/unmap functions
    - _Requirements: 6.1, 6.2, 6.3_

  - [x] 10.2 Write property test for interop data round-trip
    - **Property 8: CUDA-GL Interop Data Round-Trip**
    - **Validates: Requirements 6.2**

- [x] 11. Implement Renderer
  - [x] 11.1 Create OpenGL shader programs
    - Implement vertex shader with point size calculation
    - Implement fragment shader with depth-based coloring
    - _Requirements: 7.1, 7.4_

  - [x] 11.2 Implement Renderer class
    - Create VAO/VBO setup
    - Implement render loop
    - Add point size and color mode configuration
    - _Requirements: 7.1, 7.5_

  - [x] 11.3 Implement Camera controller
    - Create Camera class with view/projection matrices
    - Implement rotation, zoom, pan controls
    - _Requirements: 7.2_

  - [x] 11.4 Write property test for camera transformation
    - **Property 9: Camera Transformation Correctness**
    - **Validates: Requirements 7.2**

  - [x] 11.5 Write property test for color mapping
    - **Property 10: Color Mapping Correctness**
    - **Validates: Requirements 7.4**

- [x] 12. Checkpoint - Verify rendering pipeline
  - Ensure all tests pass, ask the user if questions arise.

- [x] 13. Implement ParticleSystem and Simulation Control
  - [x] 13.1 Implement ParticleSystem class
    - Integrate all components (ForceCalculator, Integrator, Interop)
    - Implement update loop
    - _Requirements: 1.1, 1.2_

  - [x] 13.2 Implement simulation control features
    - Implement pause/resume/reset
    - Implement runtime parameter adjustment
    - Implement force method switching
    - _Requirements: 8.1, 8.2, 8.3_

  - [x] 13.3 Implement state serialization
    - Implement save state to file
    - Implement load state from file
    - _Requirements: 8.4_

  - [x] 13.4 Write property test for pause/resume state preservation
    - **Property 11: Pause/Resume State Preservation**
    - **Validates: Requirements 8.1**

  - [x] 13.5 Write property test for save/load round-trip
    - **Property 12: Save/Load State Round-Trip**
    - **Validates: Requirements 8.4**

- [x] 14. Implement Error Handling
  - [x] 14.1 Implement CUDA error handling
    - Create CUDA_CHECK macro
    - Create CudaException class
    - _Requirements: 10.1_

  - [x] 14.2 Implement OpenGL error handling
    - Create checkGLError function
    - Create OpenGLException class
    - _Requirements: 10.2_

  - [x] 14.3 Implement resource validation
    - Implement GPU memory validation
    - Implement input parameter validation
    - _Requirements: 10.3, 10.4_

  - [x] 14.4 Write property test for input validation robustness
    - **Property 13: Input Validation Robustness**
    - **Validates: Requirements 10.4**

- [x] 15. Implement Main Application
  - [x] 15.1 Create main application entry point
    - Initialize GLFW window
    - Initialize CUDA context
    - Create simulation and renderer instances
    - _Requirements: 7.3_

  - [x] 15.2 Implement UI overlay
    - Display particle count, FPS, simulation time
    - Add keyboard/mouse input handling
    - _Requirements: 7.3, 8.1_

- [x] 16. Final Checkpoint - Full system integration
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- All tasks including property tests are required for comprehensive validation
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- Property tests validate universal correctness properties
- Unit tests validate specific examples and edge cases
- C++ with CUDA is the implementation language
- Google Test + RapidCheck is the testing framework
