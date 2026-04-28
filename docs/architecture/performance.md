---
layout: default
title: Performance
parent: Documentation
nav_order: 5
---

# Performance Guide

Optimization strategies and performance tuning for the N-Body Particle Simulation System.

---

## 📑 Table of Contents

1. [Benchmarks](#benchmarks)
2. [GPU Optimization](#gpu-optimization)
3. [Memory Optimization](#memory-optimization)
4. [Algorithm Tuning](#algorithm-tuning)
5. [Profiling Tools](#profiling-tools)
6. [Troubleshooting](#troubleshooting)

---

## Benchmarks

### Supported Benchmark Workflow

The repository now includes a dedicated `nbody_benchmarks` executable plus `./scripts/benchmark.sh` for non-interactive, headless benchmark runs.

```bash
./scripts/build.sh
./scripts/benchmark.sh
./scripts/benchmark.sh serialization.round_trip build/benchmark-results.json
```

With `-DNBODY_ENABLE_PROFILING=ON`, benchmark output also includes named phase timings such as `serialization.save`, `serialization.load`, `simulation.update`, or force-specific phases when those surfaces are compiled in.

### Structured Output

Benchmark runs emit machine-readable JSON with:

- benchmark name
- force method
- particle count
- iteration count
- numeric metrics
- numeric tuning parameters
- optional phase timing samples

### Performance Targets

| Particles | Target FPS | Algorithm |
|-----------|------------|-----------|
| 10,000 | 60+ | Direct N² |
| 100,000 | 60+ | Barnes-Hut |
| 1,000,000 | 30+ | Barnes-Hut / Spatial Hash |

### Reference Hardware

| Component | Specification |
|-----------|---------------|
| GPU | NVIDIA RTX 3080 (10GB) |
| CUDA | 12.2 |
| CPU | AMD Ryzen 9 5900X |
| RAM | 32GB DDR4-3600 |

### Measured Performance

#### Frame Rates (FPS)

| Particles | Direct N² | Barnes-Hut (θ=0.5) | Spatial Hash |
|-----------|-----------|-------------------|--------------|
| 1,000 | 60+ | 60+ | 60+ |
| 10,000 | 60+ | 60+ | 60+ |
| 50,000 | ~30 | 60+ | 60+ |
| 100,000 | ~8 | 60+ | 60+ |
| 500,000 | <1 | ~45 | 60+ |
| 1,000,000 | N/A | ~25 | 60+ |

#### Memory Usage

| Particles | Particle Data | Barnes-Hut | Spatial Hash | Total |
|-----------|---------------|------------|--------------|-------|
| 100K | ~5 MB | ~10 MB | ~2 MB | ~17 MB |
| 1M | ~50 MB | ~100 MB | ~20 MB | ~170 MB |
| 10M | ~500 MB | ~1 GB | ~200 MB | ~1.7 GB |

---

## GPU Optimization

### 1. Thread Block Size

Recommended block sizes by GPU architecture:

| Architecture | Series | Optimal Block Size |
|--------------|--------|-------------------|
| Ada Lovelace | RTX 40xx | 256 or 512 |
| Ampere | RTX 30xx | 256 |
| Turing | RTX 20xx | 256 |
| Volta | V100 | 256 or 512 |

```cpp
// Query optimal configuration
cudaDeviceProp prop;
cudaGetDeviceProperties(&prop, 0);

int max_threads = prop.maxThreadsPerBlock;
int warp_size = prop.warpSize;
int optimal_block_size = (max_threads / warp_size / 2) * warp_size;  // Usually 256
```

### 2. Shared Memory Usage

For Direct N² kernel, shared memory reduces global memory traffic by ~50%:

```cpp
// Shared memory per block
size_t shared_size = block_size * 4 * sizeof(float);  // pos_x, pos_y, pos_z, mass

// Check limits
if (shared_size > prop.sharedMemPerBlock) {
    // Reduce block size or disable shared memory tiling
}
```

### 3. Occupancy

Use CUDA Occupancy API to find optimal configuration:

```cpp
int min_grid_size, optimal_block_size;
cudaOccupancyMaxPotentialBlockSize(
    &min_grid_size, &optimal_block_size,
    computeForcesDirectKernel,
    0,  // dynamic shared memory
    0   // block size limit
);
```

### 4. Memory Coalescing

Ensure coalesced memory access:

```cpp
// GOOD: Coalesced access
int i = blockIdx.x * blockDim.x + threadIdx.x;
float x = pos_x[i];  // Thread i accesses address i

// BAD: Strided access (uncoalesced)
float x = pos_x[i * stride];  // Threads access non-consecutive addresses
```

### 5. Warp Divergence

Minimize branch divergence within warps:

```cpp
// BAD: Warp divergence
if (threadIdx.x % 2 == 0) {
    // Path A (16 threads active)
} else {
    // Path B (16 threads active)
}

// GOOD: No divergence
float result = (threadIdx.x % 2 == 0) ? value_a : value_b;
```

---

## Memory Optimization

### Data Layout

Use Structure of Arrays (SoA) instead of Array of Structures (AoS):

```cpp
// BAD: AoS (poor for GPU)
struct Particle { float x, y, z, vx, vy, vz, mass; };
Particle particles[N];

// GOOD: SoA (coalesced access)
struct ParticleData {
    float* pos_x; float* pos_y; float* pos_z;
    float* vel_x; float* vel_y; float* vel_z;
    float* mass;
};
```

### Memory Pool

Avoid frequent allocations:

```cpp
class MemoryPool {
public:
    void* allocate(size_t size) {
        if (pools_.count(size) && !pools_[size].empty()) {
            void* ptr = pools_[size].back();
            pools_[size].pop_back();
            return ptr;
        }
        void* ptr;
        cudaMalloc(&ptr, size);
        return ptr;
    }
    
    void deallocate(void* ptr, size_t size) {
        pools_[size].push_back(ptr);
    }
    
private:
    std::map<size_t, std::vector<void*>> pools_;
};
```

### Zero-Copy Rendering

CUDA-OpenGL interop eliminates CPU-GPU transfer:

```cpp
// Traditional (slow)
cudaMemcpy(h_positions, d_positions, size, cudaMemcpyDeviceToHost);
glBufferData(GL_ARRAY_BUFFER, size, h_positions, GL_DYNAMIC_DRAW);

// Zero-copy (fast)
float* d_vbo = interop.mapPositionBuffer();
updatePositionsKernel<<<grid, block>>>(d_particles, d_vbo);
interop.unmapPositionBuffer();
// VBO ready for rendering
```

---

## Algorithm Tuning

### Barnes-Hut Theta Parameter

Balance accuracy vs performance:

```cpp
// Adaptive theta based on FPS
void adaptTheta(float current_fps, float target_fps) {
    if (current_fps < target_fps * 0.9f) {
        theta = std::min(theta + 0.05f, 1.0f);  // Faster, less accurate
    } else if (current_fps > target_fps * 1.1f) {
        theta = std::max(theta - 0.02f, 0.3f);  // Slower, more accurate
    }
}
```

### Spatial Hash Cell Size

Optimal cell size equals cutoff radius:

```cpp
float optimal_cell_size = cutoff_radius;

// For non-uniform distributions, slightly larger
float cell_size = cutoff_radius * 1.2f;
```

### Time Step Selection

Estimate maximum stable timestep:

```cpp
float estimateMaxTimeStep(const ParticleData* d_particles, float softening) {
    // Find maximum velocity and acceleration
    float v_max = computeMaxVelocity(d_particles);
    float a_max = computeMaxAcceleration(d_particles);
    
    // Stability conditions
    float dt_pos = softening / v_max;           // Position change limit
    float dt_vel = sqrt(softening / a_max);     // Velocity change limit
    
    // Use safety factor
    return std::min(dt_pos, dt_vel) * 0.5f;
}
```

---

## Profiling Tools

### NVIDIA Nsight Systems

System-level performance analysis:

```bash
nsys profile --stats=true -o report ./nbody_sim 100000
nsys-ui report.nsys-rep  # Open in GUI
```

**Key metrics:**
- GPU utilization
- CPU-GPU synchronization overhead
- Kernel execution time
- Memory transfer time

### NVIDIA Nsight Compute

Kernel-level detailed analysis:

```bash
ncu --set full -o report.ncu-rep ./nbody_sim 100000
ncu-ui report.ncu-rep  # Open in GUI
```

**Key metrics:**
- Occupancy (%)
- Memory throughput
- Compute throughput
- Instruction mix
- Warp stall reasons

### Built-in Performance Counter

```cpp
class PerformanceCounter {
public:
    void start() {
        cudaEventRecord(start_);
    }
    
    void stop() {
        cudaEventRecord(stop_);
        cudaEventSynchronize(stop_);
        
        float ms;
        cudaEventElapsedTime(&ms, start_, stop_);
        times_.push_back(ms);
    }
    
    float getAverageFPS() const {
        float avg = std::accumulate(times_.begin(), times_.end(), 0.0f) 
                   / times_.size();
        return 1000.0f / avg;
    }
    
private:
    cudaEvent_t start_, stop_;
    std::vector<float> times_;
};
```

---

## Troubleshooting

### Low GPU Utilization

**Symptoms:** GPU utilization < 50%

**Solutions:**
1. Increase block size
2. Reduce synchronization points
3. Use multiple CUDA streams
4. Check for CPU bottlenecks

### Memory Bandwidth Bottleneck

**Symptoms:** Low compute throughput, high memory throughput

**Solutions:**
1. Use shared memory tiling
2. Ensure coalesced access
3. Consider algorithm change
4. Reduce precision (float16)

### Register Spilling

**Symptoms:** Sudden performance drop

**Check with:** `ncu --metrics launch_stats`

**Solutions:**
1. Reduce local variables
2. Use `__launch_bounds__`:
   ```cpp
   __global__ void __launch_bounds__(256, 4) kernel(...) { }
   ```

### Tree Build Overhead (Barnes-Hut)

**Symptoms:** Slower than Direct N² at small N

**Solutions:**
```cpp
// Auto-switch based on particle count
if (particle_count < 10000) {
    useDirectMethod();
} else {
    useBarnesHut();
}
```

---

## Optimization Checklist

Before profiling, ensure:

- [ ] Release build (`-O3 -DNDEBUG`)
- [ ] Fast math enabled (`-use_fast_math`)
- [ ] Correct architecture (`-arch=sm_86`)
- [ ] Latest GPU drivers
- [ ] No debug output in hot paths

Optimization priority:

1. Choose correct algorithm
2. Optimize memory access patterns
3. Tune thread block size
4. Use shared memory
5. Reduce synchronization
6. Micro-optimizations

---

## References

- [CUDA Best Practices Guide](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/)
- [CUDA Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/)
- [Nsight Documentation](https://docs.nvidia.com/nsight-systems/)
