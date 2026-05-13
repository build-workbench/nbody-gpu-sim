# Integrator API

Velocity Verlet integration.

## Class

```cpp
namespace nbody {

class Integrator {
public:
    void integrate(ParticleData& data, float dt);
    
    // Energy calculation
    double computeKineticEnergy(const ParticleData& data) const;
    double computePotentialEnergy(const ParticleData& data) const;
};

} // namespace nbody
```

## Velocity Verlet

The integrator uses the symplectic Velocity Verlet method:

1. x(t+Δt) = x(t) + v(t)Δt + ½a(t)Δt²
2. Compute a(t+Δt)
3. v(t+Δt) = v(t) + ½[a(t) + a(t+Δt)]Δt
