# Major Refactoring - v2.0.0

Date: 2026-03-09

## Critical Bug Fixes

### atomicMin/atomicMax for negative floats (Barnes-Hut bounding box)
- `computeBoundingBoxKernel` used `atomicMin(reinterpret_cast<int*>(...), __float_as_int(...))` which gives **wrong results for negative positions** due to IEEE 754 sign-magnitude vs two's-complement mismatch
- Replaced with CAS-based `atomicMinFloat`/`atomicMaxFloat` that correctly handle all sign combinations
- This was causing incorrect octree construction when particles had negative coordinates

### SpatialHashGrid uninitialized pointers
- Constructor did not initialize `d_cell_start_`, `d_cell_end_`, `d_cell_counts_` to nullptr
- Destructor called `cudaFree()` on these uninitialized pointers — **undefined behavior**
- Fixed: all device pointers now initialized to nullptr in constructor

## Performance Optimizations

### SpatialHashGrid bounding box: GPU reduction instead of CPU roundtrip
- `computeBoundingBox()` was copying ALL particle positions to host memory, computing min/max on CPU, then discarding
- For 1M particles this was ~12MB of unnecessary GPU→CPU transfer every frame
- Replaced with the GPU reduction kernel already available from Barnes-Hut module

### Integrator scratch buffer pre-allocation
- `computeKineticEnergy`/`computePotentialEnergy` were calling `cudaMalloc`/`cudaFree` on every invocation
- Added persistent `d_scratch_` buffer with `ensureScratchBuffer()` — allocates once, reuses across calls
- Added `Integrator` destructor for proper cleanup

### CUDA_CHECK_KERNEL: conditional synchronization
- The macro unconditionally called `cudaDeviceSynchronize()` after every kernel launch, **serializing all GPU work**
- Now only synchronizes in debug builds (`#ifndef NDEBUG`), uses async `cudaGetLastError()` in release builds

## Build System

### CMakeLists.txt modernization
- Added project `VERSION 2.0.0`
- `CMAKE_EXPORT_COMPILE_COMMANDS ON` for IDE/clangd support
- CUDA architecture auto-detection (`native` on CMake 3.24+, fallback to common archs)
- Replaced global `include_directories()` with proper `target_include_directories()` with generator expressions
- Added `nbody::nbody_lib` ALIAS target
- Added compiler warnings (`-Wall -Wextra` / `/W4`)
- `gtest_force_shared_crt` for MSVC compatibility
- Removed unused RapidCheck dependency
- Tests wrapped in `NBODY_BUILD_TESTS` option
