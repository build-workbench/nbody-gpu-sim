#pragma once

#include "nbody/particle_data.hpp"
#include "nbody/types.hpp"

namespace nbody::test {

// RAII fixture: releases device and host buffers even when an assertion
// throws mid-test (rapidcheck property failures throw, which used to leak
// every allocation made before the failure).
struct ScopedParticleData {
  ParticleData device;
  ParticleData host;

  explicit ScopedParticleData(size_t count) {
    ParticleDataManager::allocateDevice(device, count);
    ParticleDataManager::allocateHost(host, count);
  }

  ~ScopedParticleData() {
    ParticleDataManager::freeDevice(device);
    ParticleDataManager::freeHost(host);
  }

  ScopedParticleData(const ScopedParticleData&) = delete;
  ScopedParticleData& operator=(const ScopedParticleData&) = delete;
};

// Set a single particle's state on the host buffer.
inline void setParticle(ParticleData& h, size_t i, Vec3 pos, Vec3 vel, float mass) {
  h.pos_x[i] = pos.x;
  h.pos_y[i] = pos.y;
  h.pos_z[i] = pos.z;
  h.vel_x[i] = vel.x;
  h.vel_y[i] = vel.y;
  h.vel_z[i] = vel.z;
  h.mass[i] = mass;
}

// Host-only variant for initializer tests that never touch the device.
struct ScopedHostParticleData {
  ParticleData data;

  explicit ScopedHostParticleData(size_t count) { ParticleDataManager::allocateHost(data, count); }

  ~ScopedHostParticleData() { ParticleDataManager::freeHost(data); }

  ScopedHostParticleData(const ScopedHostParticleData&) = delete;
  ScopedHostParticleData& operator=(const ScopedHostParticleData&) = delete;
};

}  // namespace nbody::test
