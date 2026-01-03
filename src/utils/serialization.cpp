#include "nbody/serialization.hpp"
#include "nbody/error_handling.hpp"
#include <fstream>
#include <cstring>

namespace nbody {

void Serializer::save(const std::string& filename, const SimulationState& state) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    save(file, state);
}

SimulationState Serializer::load(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }
    return load(file);
}

void Serializer::save(std::ostream& out, const SimulationState& state) {
    writeHeader(out, state);
    
    // Write particle data
    writeFloatArray(out, state.pos_x);
    writeFloatArray(out, state.pos_y);
    writeFloatArray(out, state.pos_z);
    writeFloatArray(out, state.vel_x);
    writeFloatArray(out, state.vel_y);
    writeFloatArray(out, state.vel_z);
    writeFloatArray(out, state.mass);
}

SimulationState Serializer::load(std::istream& in) {
    FileHeader header = readHeader(in);
    
    SimulationState state;
    state.particle_count = header.particle_count;
    state.simulation_time = header.simulation_time;
    state.dt = header.dt;
    state.G = header.G;
    state.softening = header.softening;
    state.force_method = static_cast<ForceMethod>(header.force_method);
    
    // Read particle data
    state.pos_x = readFloatArray(in, state.particle_count);
    state.pos_y = readFloatArray(in, state.particle_count);
    state.pos_z = readFloatArray(in, state.particle_count);
    state.vel_x = readFloatArray(in, state.particle_count);
    state.vel_y = readFloatArray(in, state.particle_count);
    state.vel_z = readFloatArray(in, state.particle_count);
    state.mass = readFloatArray(in, state.particle_count);
    
    return state;
}

bool Serializer::validateFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    return validateStream(file);
}

bool Serializer::validateStream(std::istream& in) {
    try {
        FileHeader header = readHeader(in);
        return header.magic == NBODY_MAGIC && header.version == NBODY_VERSION;
    } catch (...) {
        return false;
    }
}

void Serializer::writeHeader(std::ostream& out, const SimulationState& state) {
    FileHeader header;
    header.magic = NBODY_MAGIC;
    header.version = NBODY_VERSION;
    header.particle_count = state.particle_count;
    header.simulation_time = state.simulation_time;
    header.dt = state.dt;
    header.G = state.G;
    header.softening = state.softening;
    header.force_method = static_cast<uint32_t>(state.force_method);
    std::memset(header.reserved, 0, sizeof(header.reserved));
    
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
}

FileHeader Serializer::readHeader(std::istream& in) {
    FileHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (header.magic != NBODY_MAGIC) {
        throw std::runtime_error("Invalid file format: wrong magic number");
    }
    
    if (header.version != NBODY_VERSION) {
        throw std::runtime_error("Unsupported file version");
    }
    
    return header;
}

void Serializer::writeFloatArray(std::ostream& out, const std::vector<float>& data) {
    out.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(float));
}

std::vector<float> Serializer::readFloatArray(std::istream& in, size_t count) {
    std::vector<float> data(count);
    in.read(reinterpret_cast<char*>(data.data()), count * sizeof(float));
    return data;
}

// SimulationState serialization methods
void SimulationState::serialize(std::ostream& out) const {
    Serializer::save(out, *this);
}

SimulationState SimulationState::deserialize(std::istream& in) {
    return Serializer::load(in);
}

} // namespace nbody
