#pragma once

#include "nbody/types.hpp"
#include "nbody/particle_system.hpp"
#include <iostream>
#include <fstream>
#include <vector>

namespace nbody {

// Binary serialization format
// Header: magic number, version, particle count, parameters
// Data: positions, velocities, masses

constexpr uint32_t NBODY_MAGIC = 0x4E424F44;  // "NBOD"
constexpr uint32_t NBODY_VERSION = 1;

struct FileHeader {
    uint32_t magic;
    uint32_t version;
    uint64_t particle_count;
    float simulation_time;
    float dt;
    float G;
    float softening;
    uint32_t force_method;
    uint32_t reserved[4];  // For future use
};

class Serializer {
public:
    // Save simulation state to file
    static void save(const std::string& filename, const SimulationState& state);
    
    // Load simulation state from file
    static SimulationState load(const std::string& filename);
    
    // Save to stream
    static void save(std::ostream& out, const SimulationState& state);
    
    // Load from stream
    static SimulationState load(std::istream& in);
    
    // Validate file format
    static bool validateFile(const std::string& filename);
    static bool validateStream(std::istream& in);
    
private:
    static void writeHeader(std::ostream& out, const SimulationState& state);
    static FileHeader readHeader(std::istream& in);
    static void writeFloatArray(std::ostream& out, const std::vector<float>& data);
    static std::vector<float> readFloatArray(std::istream& in, size_t count);
};

} // namespace nbody
