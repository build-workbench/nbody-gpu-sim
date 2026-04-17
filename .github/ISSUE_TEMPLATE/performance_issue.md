---
name: Performance Issue
about: Report performance problems or suggest optimizations
title: '[PERF] '
labels: 'performance'
assignees: ''
---

## Performance Issue Description

Describe the performance issue you're experiencing.

## Environment

**Hardware:**
- GPU: [e.g., NVIDIA RTX 3080]
- VRAM: [e.g., 10GB]
- CPU: [e.g., AMD Ryzen 9 5900X]
- RAM: [e.g., 32GB]

**Software:**
- OS: [e.g., Ubuntu 22.04]
- CUDA Version: [e.g., 12.1]
- GPU Driver: [e.g., 530.41.03]
- Project Version: [e.g., 2.0.0]

## Benchmark Results

### Current Performance

| Particles | Algorithm | FPS | Expected FPS |
|-----------|-----------|-----|--------------|
| | | | |

### Profiling Data

```
Paste nvprof or Nsight profiling results here
```

## Reproduction Steps

```bash
# Commands used for benchmarking
./nbody_sim 100000 --algorithm barnes-hut --benchmark
```

## Expected Performance

What performance do you expect, and why? (Reference benchmarks, similar projects, etc.)

## Potential Optimizations

If you have ideas for potential optimizations, please share them:

1. ...
2. ...

## Additional Context

Add any other context about the performance issue here.
