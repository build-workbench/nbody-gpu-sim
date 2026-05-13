# Benchmark Methodology

How performance is measured.

## Test Environment

- GPU: NVIDIA RTX 3080
- CUDA: 11.8
- OS: Ubuntu 22.04
- Build: Release with -O3

## Metrics

| Metric | Description |
|--------|-------------|
| FPS | Frames per second |
| Step Time | Milliseconds per simulation step |
| Memory | GPU memory usage |
| Occupancy | GPU kernel occupancy |

## Procedure

1. Warm-up: Run 100 steps
2. Measure: Run 1000 steps
3. Average: Calculate mean and std dev

## Reproducibility

```bash
# Set environment
export NBODY_BENCHMARK_PARTICLES=100000
export NBODY_BENCHMARK_ITERATIONS=1000

# Run benchmark
./build/benchmark --benchmark_format=json --benchmark_out=results.json
```

## Variability

Performance may vary due to:
- GPU temperature throttling
- Background processes
- Memory fragmentation
