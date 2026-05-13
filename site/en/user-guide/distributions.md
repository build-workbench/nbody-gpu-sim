# Particle Distributions

Initial particle configurations.

## Built-in Distributions

| Distribution | Description | Use Case |
|--------------|-------------|----------|
| `SPHERICAL` | Uniform sphere | Galaxy clusters |
| `DISK` | Thin rotating disk | Spiral galaxies |
| `CUBE` | Uniform cube | General testing |
| `RANDOM` | Random positions | Stress testing |

## Custom Distribution

```cpp
ParticleData data;
data.resize(n);

for (size_t i = 0; i < n; ++i) {
    // Set positions, velocities, masses manually
    data.position_x[i] = /* custom */;
    data.velocity_x[i] = /* custom */;
    data.mass[i] = /* custom */;
}

ParticleSystem system;
system.initialize(data, method);
```
