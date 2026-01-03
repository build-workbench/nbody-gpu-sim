#pragma once

#include "nbody/types.hpp"
#include "nbody/particle_data.hpp"
#include "nbody/force_calculator.hpp"
#include "nbody/integrator.hpp"
#include "nbody/cuda_gl_interop.hpp"
#include <memory>
#include <string>

namespace nbody {

// Simulation state for serialization
struct SimulationState {
    std::vector<float> pos_x, pos_y, pos_z;
    std::vector<float> vel_x, vel_y, vel_z;
    std::vector<float> mass;
    size_t particle_count;
    float simulation_time;
    float dt;
    float G;
    float softening;
    ForceMethod force_method;
    
    // Serialization
    void serialize(std::ostream& out) const;
    static SimulationState deserialize(std::istream& in);
    
    // Comparison for testing
    bool operator==(const SimulationState& other) const;
};

// Main particle system class
class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();
    
    // Initialization
    void initialize(const SimulationConfig& config);
    void initializeWithDistribution(size_t particle_count, InitDistribution dist);
    
    // Simulation control
    void update(float dt);
    void pause() { is_paused_ = true; }
    void resume() { is_paused_ = false; }
    void reset();
    bool isPaused() const { return is_paused_; }
    
    // Parameter adjustment
    void setForceMethod(ForceMethod method);
    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
    void setTimeStep(float dt) { dt_ = dt; }
    void setBarnesHutTheta(float theta);
    void setSpatialHashCellSize(float size);
    void setSpatialHashCutoff(float cutoff);
    
    // Getters
    ForceMethod getForceMethod() const { return force_method_; }
    float getGravitationalConstant() const { return G_; }
    float getSofteningParameter() const { return softening_; }
    float getTimeStep() const { return dt_; }
    float getSimulationTime() const { return simulation_time_; }
    size_t getParticleCount() const { return particle_count_; }
    
    // Data access
    ParticleData* getDeviceData() { return &d_particles_; }
    const ParticleData* getDeviceData() const { return &d_particles_; }
    
    // Copy data to host for inspection
    void copyToHost(ParticleData& h_particles) const;
    
    // State management
    void saveState(const std::string& filename) const;
    void loadState(const std::string& filename);
    SimulationState getState() const;
    void setState(const SimulationState& state);
    
    // Energy calculations
    float computeKineticEnergy() const;
    float computePotentialEnergy() const;
    float computeTotalEnergy() const;
    
    // CUDA-GL interop
    CudaGLInterop* getInterop() { return interop_.get(); }
    void initializeInterop();
    void updateInteropBuffer();
    
private:
    // Particle data
    ParticleData d_particles_;  // Device memory
    ParticleData h_particles_;  // Host memory (for initialization/serialization)
    size_t particle_count_;
    
    // Components
    std::unique_ptr<ForceCalculator> force_calculator_;
    std::unique_ptr<Integrator> integrator_;
    std::unique_ptr<CudaGLInterop> interop_;
    
    // Simulation parameters
    float dt_;
    float G_;
    float softening_;
    float simulation_time_;
    ForceMethod force_method_;
    bool is_paused_;
    bool is_initialized_;
    
    // Configuration
    SimulationConfig config_;
    
    // Internal methods
    void allocateMemory(size_t count);
    void freeMemory();
    void createForceCalculator();
};

} // namespace nbody
