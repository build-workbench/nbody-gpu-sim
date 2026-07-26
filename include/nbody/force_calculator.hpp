#pragma once

#include "nbody/types.hpp"
#include <memory>

namespace nbody {

class ForceCalculator {
public:
  virtual ~ForceCalculator() = default;

  virtual void computeForces(ParticleData* d_particles) = 0;
  virtual ForceMethod getMethod() const = 0;

  void setSofteningParameter(float eps) {
    softening_eps_ = eps;
    softening_eps2_ = eps * eps;
  }
  void setGravitationalConstant(float G) { G_ = G; }
  float getSofteningParameter() const noexcept { return softening_eps_; }
  float getGravitationalConstant() const noexcept { return G_; }

protected:
  float softening_eps_ = 0.01f;
  float softening_eps2_ = 0.0001f;
  float G_ = 1.0f;
};

class DirectForceCalculator : public ForceCalculator {
public:
  explicit DirectForceCalculator(int block_size = 256);
  void computeForces(ParticleData* d_particles) override;
  ForceMethod getMethod() const noexcept override { return ForceMethod::DIRECT_N2; }
  void setBlockSize(int size) { block_size_ = size; }
  int getBlockSize() const noexcept { return block_size_; }

private:
  int block_size_;
};

class BarnesHutCalculator : public ForceCalculator {
public:
  explicit BarnesHutCalculator(float theta = 0.5f);
  ~BarnesHutCalculator();
  void computeForces(ParticleData* d_particles) override;
  ForceMethod getMethod() const noexcept override { return ForceMethod::BARNES_HUT; }
  void setTheta(float theta) { theta_ = theta; }
  float getTheta() const noexcept { return theta_; }
  BarnesHutTree* getTree() noexcept { return tree_.get(); }

private:
  std::unique_ptr<BarnesHutTree> tree_;
  float theta_;
};

class SpatialHashCalculator : public ForceCalculator {
public:
  SpatialHashCalculator(float cell_size = 1.0f, float cutoff_radius = 2.0f);
  ~SpatialHashCalculator();
  void computeForces(ParticleData* d_particles) override;
  ForceMethod getMethod() const noexcept override { return ForceMethod::SPATIAL_HASH; }
  void setCellSize(float size) { cell_size_ = size; }
  void setCutoffRadius(float radius) { cutoff_radius_ = radius; }
  float getCellSize() const noexcept { return cell_size_; }
  float getCutoffRadius() const noexcept { return cutoff_radius_; }
  SpatialHashGrid* getGrid() noexcept { return grid_.get(); }

private:
  std::unique_ptr<SpatialHashGrid> grid_;
  float cell_size_;
  float cutoff_radius_;
};

std::unique_ptr<ForceCalculator> createForceCalculator(ForceMethod method,
                                                       const SimulationConfig& config);

// CPU reference for testing GPU implementations
Vec3 computeGravitationalForceCPU(const Vec3& p1, const Vec3& p2, float m1, float m2, float G,
                                  float eps);

}  // namespace nbody
