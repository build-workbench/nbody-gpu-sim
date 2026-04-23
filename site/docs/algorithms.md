---
title: Algorithms
description: Force calculation algorithms explained.
---

# Force Calculation Algorithms

Three algorithms are provided for computing N-body forces, each with different time/space complexity trade-offs.

---

## Algorithm Comparison

| Algorithm | Time | Space | Best For |
|-----------|------|-------|----------|
| **Direct N²** | O(N²) | O(1) | Small systems, validation |
| **Barnes-Hut** | O(N log N) | O(N) | Large gravitational systems |
| **Spatial Hash** | O(N) | O(N) | Short-range forces |

---

## Direct N² Algorithm

### Overview

Computes exact pairwise gravitational forces between all particles.

```cpp
for each particle i:
    for each particle j (j ≠ i):
        r = position[j] - position[i]
        dist = |r| + softening
        force = G * mass[i] * mass[j] / dist²
        acceleration[i] += force * r / dist
```

### Time Complexity

- **Force calculation**: O(N²)
- **Memory**: O(1) per thread (uses shared memory caching)

### When to Use

- Particle count < 10,000
- Accuracy validation
- Benchmarking other methods

---

## Barnes-Hut Algorithm

### Overview

Uses hierarchical octree to approximate forces from distant particle groups.

### Key Idea

If a group of particles is sufficiently far away (θ < s/d), treat them as a single point mass at their center of mass.

```
          ┌───────────────┐
          │     Root      │
          │   Center of   │  θ = s/d determines approximation
          │     Mass      │
          └───────┬───────┘
         ┌────────┼────────┐
    ┌────┴────┐ ┌┴┐ ┌───────┴──────┐
    │ NW      │ │ │ │ NE           │  Leaf = single particle
    │ (4)     │ │ │ │ (4)          │  Node = center of mass
    └─────────┘ └─┘ └──────────────┘
```

### Time Complexity

- **Tree construction**: O(N log N)
- **Force calculation**: O(N log N)
- **Memory**: O(N)

### θ Parameter

| θ Value | Accuracy | Speed |
|---------|----------|-------|
| 0.3 | High | Slower |
| 0.5 | Medium | Balanced |
| 0.7 | Lower | Faster |

### When to Use

- Large-scale gravitational simulation
- Particle count > 10,000
- Long-range forces

---

## Spatial Hash Algorithm

### Overview

Partitions 3D space into uniform grid cells, only computing forces between particles in neighboring cells.

```
┌─────┬─────┬─────┐
│  ●  │     │  ●  │  Particle only checks
├─────┼─────┼─────┤  neighbors in adjacent
│     │  ●  │     │  cells (including diagonal)
├─────┼─────┼─────┤
│  ●  │     │  ●  │
└─────┴─────┴─────┘
        ↓
   Cutoff radius
```

### Time Complexity

- **Grid construction**: O(N)
- **Force calculation**: O(N) per particle (constant neighbors)
- **Memory**: O(N)

### When to Use

- Short-range forces (cutoff radius)
- Molecular dynamics
- Very large particle counts (> 100K)

---

## Algorithm Selection Guide

```
Particle Count │ Algorithm
───────────────┼─────────────
    < 10K      │ Direct N²
  10K - 100K   │ Barnes-Hut
   > 100K      │ Spatial Hash (short-range)
   > 100K      │ Barnes-Hut (long-range)
```

Runtime switching:
```bash
./nbody_sim
# Press 1: Direct N²
# Press 2: Barnes-Hut
# Press 3: Spatial Hash
```
