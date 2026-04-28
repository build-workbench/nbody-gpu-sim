---
layout: docs
lang: en
title: Performance
description: Optimization strategies and profiling for N-Body Simulation.
---

# Performance Guide

Optimization strategies and profiling for N-Body Simulation.

---
layout: docs
lang: en

## Performance Metrics

### Benchmarks (RTX 3080)

| Particles | Direct N² | Barnes-Hut | Spatial Hash |
|-----------|-----------|------------|--------------|
| 10K | 60+ FPS | 120+ FPS | 120+ FPS |
| 100K | ~10 FPS | 60+ FPS | 90+ FPS |
| 1M | <1 FPS | 25+ FPS | 60+ FPS |
| 5M | N/A | ~5 FPS | 15+ FPS |

### Memory Footprint

| Configuration | Memory Usage |
|---------------|--------------|
| Base (position, velocity, mass) | ~52 bytes/particle |
| Barnes-Hut tree | +~32 bytes/particle |
| Spatial hash grid | +~16 bytes/particle |
| **1M particles total** | **~52-84 MB** |

---
layout: docs
lang: en

## Optimization Strategies

### 1. Algorithm Selection

Choose the right algorithm for your particle count:

```cpp
if (n < 10000) {
    method = ForceMethod::DIRECT;
} else if (long_range_forces) {
    method = ForceMethod::BARNES_HUT;
} else {
    method = ForceMethod::SPATIAL_HASH;
}
```

### 2. Time Step Tuning

- Smaller dt = more accurate, slower
- Larger dt = faster, potential instability
- Typical range: 0.0001 to 0.01

### 3. Softening Parameter

Prevents numerical singularities:

```cpp
// Too small: numerical instability
// Too large: distorts physics at small scales
config.softening = 0.01f;  // Good default
```

### 4. Barnes-Hut Theta

```cpp
// Lower θ = more accurate, slower
config.theta = 0.3f;  // High accuracy
config.theta = 0.5f;  // Balanced
config.theta = 0.7f;  // Fast, lower accuracy
```

### 5. GPU Architecture

Compile with your GPU's compute capability:

```bash
# RTX 30xx (Ampere)
cmake .. -DCMAKE_CUDA_ARCHITECTURES=86

# RTX 20xx (Turing)
cmake .. -DCMAKE_CUDA_ARCHITECTURES=75

# GTX 10xx (Pascal)
cmake .. -DCMAKE_CUDA_ARCHITECTURES=61
```

---
layout: docs
lang: en

## Profiling

### Nsight Compute

```bash
ncu --set full ./nbody_sim 100000
```

### Nsight Systems

```bash
nsys profile -o report ./nbody_sim 100000
nsys ui report.nsys-rep
```

### Built-in Timing

Enable in config for frame time output:

```cpp
config.enable_profiling = true;
```

---
layout: docs
lang: en

## Best Practices

1. **Always build Release mode**: `cmake -DCMAKE_BUILD_TYPE=Release`
2. **Use appropriate algorithm**: See selection guide above
3. **Monitor VRAM**: Use `nvidia-smi` to check memory usage
4. **Batch multiple simulations**: Amortize initialization cost
5. **Profile before optimizing**: Don't guess, measure
