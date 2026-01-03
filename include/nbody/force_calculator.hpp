#pragma once

#include "nbody/types.hpp"
#include <memory>

namespace nbody {

// Abstract base class for force calculators
class ForceCalculator {
public:
    virtual ~ForceCalculator() = default;
    
    // Compute forces for all particles
    virtual void computeForces(ParticleData* d_particles) = 0;
    
    // Get the force method type
    virtual ForceMethod getMethod() const = 0;
    
    // Setters
    void setSofteningParameter(float eps) { softening_eps_ = eps; softening_eps2_ = eps * eps; }
    void setGravitationalConstant(float G) { G_ = G; }
    
    // Getters
    float getSofteningParameter() const { return softening_eps_; }
    float getGravitationalConstant() const { return G_; }
    
protected:
    float softening_eps_ = 0.01f;
    float softening_eps2_ = 0.0001f;  // eps^2 for efficiency
    float G_ = 1.0f;
};

// Direct O(N²) force calculator
class DirectForceCalculator : public ForceCalculator {
public:
    DirectForceCalculator(int block_size = 256);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override { return ForceMethod::DIRECT_N2; }
    
    void setBlockSize(int size) { block_size_ = size; }
    int getBlockSize() const { return block_size_; }
    
private:
    int block_size_;
};

// Barnes-Hut O(N log N) force calculator
class BarnesHutCalculator : public ForceCalculator {
public:
    BarnesHutCalculator(float theta = 0.5f);
    ~BarnesHutCalculator();
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override { return ForceMethod::BARNES_HUT; }
    
    void setTheta(float theta) { theta_ = theta; }
    float getTheta() const { return theta_; }
    
    // Access to tree for testing
    BarnesHutTree* getTree() { return tree_.get(); }
    
private:
    std::unique_ptr<BarnesHutTree> tree_;
    float theta_;
};

// Spatial Hash O(N) force calculator (for short-range forces)
class SpatialHashCalculator : public ForceCalculator {
public:
    SpatialHashCalculator(float cell_size = 1.0f, float cutoff_radius = 2.0f);
    ~SpatialHashCalculator();
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override { return ForceMethod::SPATIAL_HASH; }
    
    void setCellSize(float size) { cell_size_ = size; }
    void setCutoffRadius(float radius) { cutoff_radius_ = radius; }
    
    float getCellSize() const { return cell_size_; }
    float getCutoffRadius() const { return cutoff_radius_; }
    
    // Access to grid for testing
    SpatialHashGrid* getGrid() { return grid_.get(); }
    
private:
    std::unique_ptr<SpatialHashGrid> grid_;
    float cell_size_;
    float cutoff_radius_;
};

// Factory function to create force calculator
std::unique_ptr<ForceCalculator> createForceCalculator(ForceMethod method,
                                                        const SimulationConfig& config);

// CPU reference implementation for testing
Vec3 computeGravitationalForceCPU(const Vec3& p1, const Vec3& p2,
                                   float m1, float m2, float G, float eps);

} // namespace nbody
