#pragma once

#include "nbody/force_calculator.hpp"
#include "nbody/integrator.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/simulation_state.hpp"
#include "nbody/types.hpp"
#include <memory>
#include <string>

namespace nbody {

// Not thread-safe. CUDA-GL interop buffer mapping is handled automatically
// by update() and render().
class ParticleSystem {
public:
  ParticleSystem();
  ~ParticleSystem();

  // Non-copyable, non-movable: ParticleData holds raw device/host pointers
  // without ownership semantics, so a default move would shallow-copy the
  // pointers and double-free in both destructors.
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;
  ParticleSystem(ParticleSystem&&) = delete;
  ParticleSystem& operator=(ParticleSystem&&) = delete;

  void initialize(const SimulationConfig& config);
  void initializeWithDistribution(size_t particle_count, InitDistribution dist);

  void update(float dt);
  void pause() { is_paused_ = true; }
  void resume() { is_paused_ = false; }
  void reset();
  bool isPaused() const noexcept { return is_paused_; }

  void setForceMethod(ForceMethod method);
  void setGravitationalConstant(float G);
  void setSofteningParameter(float eps);
  void setTimeStep(float dt);
  void setBarnesHutTheta(float theta);
  void setSpatialHashCellSize(float size);
  void setSpatialHashCutoff(float cutoff);

  ForceMethod getForceMethod() const noexcept { return force_method_; }
  float getGravitationalConstant() const noexcept { return G_; }
  float getSofteningParameter() const noexcept { return softening_; }
  float getTimeStep() const noexcept { return dt_; }
  float getSimulationTime() const noexcept { return simulation_time_; }
  size_t getParticleCount() const noexcept { return particle_count_; }

  ParticleData* getDeviceData() { return &d_particles_; }
  const ParticleData* getDeviceData() const { return &d_particles_; }
  void copyToHost(ParticleData& h_particles) const;

  void saveState(const std::string& filename) const;
  void loadState(const std::string& filename);
  SimulationState getState() const;
  void setState(const SimulationState& state);

  float computeKineticEnergy() const;
  float computePotentialEnergy() const;
  float computeTotalEnergy() const;

  CudaGLInterop* getInterop() noexcept { return interop_.get(); }
  const CudaGLInterop* getInterop() const noexcept { return interop_.get(); }
  void initializeInterop();
  void updateInteropBuffer();

private:
  ParticleData d_particles_;
  ParticleData h_particles_;
  size_t particle_count_;

  std::unique_ptr<ForceCalculator> force_calculator_;
  std::unique_ptr<Integrator> integrator_;
  std::unique_ptr<CudaGLInterop> interop_;

  float dt_;
  float G_;
  float softening_;
  float simulation_time_;
  ForceMethod force_method_;
  bool is_paused_;
  bool is_initialized_;

  SimulationConfig config_;

  void allocateMemory(size_t count);
  void freeMemory();
  void createForceCalculator();
  void finishInitialization();
};

}  // namespace nbody
