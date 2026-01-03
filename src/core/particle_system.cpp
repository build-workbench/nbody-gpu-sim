#include "nbody/particle_system.hpp"
#include "nbody/serialization.hpp"
#include "nbody/error_handling.hpp"
#include <fstream>
#include <cstring>

namespace nbody {

ParticleSystem::ParticleSystem()
    : particle_count_(0), dt_(0.001f), G_(1.0f), softening_(0.01f),
      simulation_time_(0.0f), force_method_(ForceMethod::DIRECT_N2),
      is_paused_(false), is_initialized_(false) {}

ParticleSystem::~ParticleSystem() {
    freeMemory();
}

void ParticleSystem::allocateMemory(size_t count) {
    particle_count_ = count;
    ParticleDataManager::allocateDevice(d_particles_, count);
    ParticleDataManager::allocateHost(h_particles_, count);
}

void ParticleSystem::freeMemory() {
    if (is_initialized_) {
        ParticleDataManager::freeDevice(d_particles_);
        ParticleDataManager::freeHost(h_particles_);
        is_initialized_ = false;
    }
}

void ParticleSystem::initialize(const SimulationConfig& config) {
    config_ = config;
    dt_ = config.dt;
    G_ = config.G;
    softening_ = config.softening;
    force_method_ = config.force_method;
    
    freeMemory();
    allocateMemory(config.particle_count);
    
    // Initialize particles based on distribution
    switch (config.init_distribution) {
        case InitDistribution::UNIFORM: {
            UniformDistParams params;
            params.min_bounds = Vec3(-10, -10, -10);
            params.max_bounds = Vec3(10, 10, 10);
            ParticleInitializer::initUniform(h_particles_, params);
            break;
        }
        case InitDistribution::SPHERICAL: {
            SphericalDistParams params;
            params.center = Vec3(0, 0, 0);
            params.radius = 10.0f;
            ParticleInitializer::initSpherical(h_particles_, params);
            break;
        }
        case InitDistribution::DISK: {
            DiskDistParams params;
            params.center = Vec3(0, 0, 0);
            params.radius = 10.0f;
            params.thickness = 1.0f;
            params.rotation_speed = 0.5f;
            ParticleInitializer::initDisk(h_particles_, params);
            break;
        }
    }
    
    // Copy to device
    ParticleDataManager::copyToDevice(d_particles_, h_particles_);
    
    // Create force calculator
    createForceCalculator();
    
    // Create integrator
    integrator_ = std::make_unique<Integrator>(config.cuda_block_size);
    
    // Compute initial forces
    force_calculator_->computeForces(&d_particles_);
    
    simulation_time_ = 0.0f;
    is_initialized_ = true;
}

void ParticleSystem::initializeWithDistribution(size_t particle_count, InitDistribution dist) {
    SimulationConfig config;
    config.particle_count = particle_count;
    config.init_distribution = dist;
    initialize(config);
}

void ParticleSystem::createForceCalculator() {
    force_calculator_ = nbody::createForceCalculator(force_method_, config_);
    force_calculator_->setGravitationalConstant(G_);
    force_calculator_->setSofteningParameter(softening_);
}

void ParticleSystem::update(float dt) {
    if (!is_initialized_ || is_paused_) return;
    
    integrator_->integrate(&d_particles_, force_calculator_.get(), dt);
    simulation_time_ += dt;
    
    // Update interop buffer if initialized
    if (interop_) {
        updateInteropBuffer();
    }
}

void ParticleSystem::reset() {
    if (!is_initialized_) return;
    
    // Re-initialize with same config
    initialize(config_);
}

void ParticleSystem::setForceMethod(ForceMethod method) {
    if (force_method_ != method) {
        force_method_ = method;
        config_.force_method = method;
        createForceCalculator();
    }
}

void ParticleSystem::setGravitationalConstant(float G) {
    G_ = G;
    config_.G = G;
    if (force_calculator_) {
        force_calculator_->setGravitationalConstant(G);
    }
}

void ParticleSystem::setSofteningParameter(float eps) {
    softening_ = eps;
    config_.softening = eps;
    if (force_calculator_) {
        force_calculator_->setSofteningParameter(eps);
    }
}

void ParticleSystem::setBarnesHutTheta(float theta) {
    config_.barnes_hut_theta = theta;
    if (force_method_ == ForceMethod::BARNES_HUT) {
        auto* bh = dynamic_cast<BarnesHutCalculator*>(force_calculator_.get());
        if (bh) bh->setTheta(theta);
    }
}

void ParticleSystem::setSpatialHashCellSize(float size) {
    config_.spatial_hash_cell_size = size;
    if (force_method_ == ForceMethod::SPATIAL_HASH) {
        auto* sh = dynamic_cast<SpatialHashCalculator*>(force_calculator_.get());
        if (sh) sh->setCellSize(size);
    }
}

void ParticleSystem::setSpatialHashCutoff(float cutoff) {
    config_.spatial_hash_cutoff = cutoff;
    if (force_method_ == ForceMethod::SPATIAL_HASH) {
        auto* sh = dynamic_cast<SpatialHashCalculator*>(force_calculator_.get());
        if (sh) sh->setCutoffRadius(cutoff);
    }
}

void ParticleSystem::copyToHost(ParticleData& h_particles) const {
    ParticleDataManager::copyToHost(h_particles, d_particles_);
}

SimulationState ParticleSystem::getState() const {
    SimulationState state;
    state.particle_count = particle_count_;
    state.simulation_time = simulation_time_;
    state.dt = dt_;
    state.G = G_;
    state.softening = softening_;
    state.force_method = force_method_;
    
    // Copy particle data
    ParticleData h_data;
    ParticleDataManager::allocateHost(h_data, particle_count_);
    ParticleDataManager::copyToHost(h_data, d_particles_);
    
    state.pos_x.assign(h_data.pos_x, h_data.pos_x + particle_count_);
    state.pos_y.assign(h_data.pos_y, h_data.pos_y + particle_count_);
    state.pos_z.assign(h_data.pos_z, h_data.pos_z + particle_count_);
    state.vel_x.assign(h_data.vel_x, h_data.vel_x + particle_count_);
    state.vel_y.assign(h_data.vel_y, h_data.vel_y + particle_count_);
    state.vel_z.assign(h_data.vel_z, h_data.vel_z + particle_count_);
    state.mass.assign(h_data.mass, h_data.mass + particle_count_);
    
    ParticleDataManager::freeHost(h_data);
    
    return state;
}

void ParticleSystem::setState(const SimulationState& state) {
    freeMemory();
    allocateMemory(state.particle_count);
    
    // Copy data to host buffer
    std::copy(state.pos_x.begin(), state.pos_x.end(), h_particles_.pos_x);
    std::copy(state.pos_y.begin(), state.pos_y.end(), h_particles_.pos_y);
    std::copy(state.pos_z.begin(), state.pos_z.end(), h_particles_.pos_z);
    std::copy(state.vel_x.begin(), state.vel_x.end(), h_particles_.vel_x);
    std::copy(state.vel_y.begin(), state.vel_y.end(), h_particles_.vel_y);
    std::copy(state.vel_z.begin(), state.vel_z.end(), h_particles_.vel_z);
    std::copy(state.mass.begin(), state.mass.end(), h_particles_.mass);
    ParticleInitializer::zeroAccelerations(h_particles_);
    
    // Copy to device
    ParticleDataManager::copyToDevice(d_particles_, h_particles_);
    
    // Set parameters
    simulation_time_ = state.simulation_time;
    dt_ = state.dt;
    G_ = state.G;
    softening_ = state.softening;
    force_method_ = state.force_method;
    
    config_.particle_count = state.particle_count;
    config_.dt = state.dt;
    config_.G = state.G;
    config_.softening = state.softening;
    config_.force_method = state.force_method;
    
    createForceCalculator();
    integrator_ = std::make_unique<Integrator>(config_.cuda_block_size);
    
    // Compute initial forces
    force_calculator_->computeForces(&d_particles_);
    
    is_initialized_ = true;
}

void ParticleSystem::saveState(const std::string& filename) const {
    SimulationState state = getState();
    Serializer::save(filename, state);
}

void ParticleSystem::loadState(const std::string& filename) {
    SimulationState state = Serializer::load(filename);
    setState(state);
}

float ParticleSystem::computeKineticEnergy() const {
    if (!integrator_) return 0.0f;
    return integrator_->computeKineticEnergy(&d_particles_);
}

float ParticleSystem::computePotentialEnergy() const {
    if (!integrator_) return 0.0f;
    return integrator_->computePotentialEnergy(&d_particles_, G_, softening_);
}

float ParticleSystem::computeTotalEnergy() const {
    return computeKineticEnergy() + computePotentialEnergy();
}

void ParticleSystem::initializeInterop() {
    interop_ = std::make_unique<CudaGLInterop>();
    interop_->initialize(particle_count_);
    updateInteropBuffer();
}

void ParticleSystem::updateInteropBuffer() {
    if (interop_) {
        interop_->updatePositions(&d_particles_);
    }
}

// SimulationState implementation
bool SimulationState::operator==(const SimulationState& other) const {
    if (particle_count != other.particle_count) return false;
    if (std::abs(simulation_time - other.simulation_time) > 1e-6f) return false;
    if (std::abs(dt - other.dt) > 1e-6f) return false;
    if (std::abs(G - other.G) > 1e-6f) return false;
    if (std::abs(softening - other.softening) > 1e-6f) return false;
    if (force_method != other.force_method) return false;
    
    for (size_t i = 0; i < particle_count; i++) {
        if (std::abs(pos_x[i] - other.pos_x[i]) > 1e-6f) return false;
        if (std::abs(pos_y[i] - other.pos_y[i]) > 1e-6f) return false;
        if (std::abs(pos_z[i] - other.pos_z[i]) > 1e-6f) return false;
        if (std::abs(vel_x[i] - other.vel_x[i]) > 1e-6f) return false;
        if (std::abs(vel_y[i] - other.vel_y[i]) > 1e-6f) return false;
        if (std::abs(vel_z[i] - other.vel_z[i]) > 1e-6f) return false;
        if (std::abs(mass[i] - other.mass[i]) > 1e-6f) return false;
    }
    
    return true;
}

} // namespace nbody
