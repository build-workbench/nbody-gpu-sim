# ForceCalculator API

Interface for force calculation strategies.

## Interface

```cpp
namespace nbody {

class ForceCalculator {
public:
    virtual ~ForceCalculator() = default;
    
    virtual void compute(ParticleData& data) = 0;
    virtual ForceMethod getMethod() const = 0;
    
    // Factory method
    static std::unique_ptr<ForceCalculator> create(ForceMethod method);
};

} // namespace nbody
```

## Usage

```cpp
auto calculator = ForceCalculator::create(ForceMethod::BARNES_HUT);
calculator->compute(particle_data);
```

## Implementations

| Class | Method | Complexity |
|-------|--------|------------|
| `DirectForce` | Exact pairwise | O(N²) |
| `BarnesHutForce` | Octree approximation | O(N log N) |
| `SpatialHashForce` | Grid-based local | O(N) |
