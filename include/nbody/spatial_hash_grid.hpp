#pragma once

#include "nbody/types.hpp"
#include <vector>

namespace nbody {

// Spatial hash grid for O(N) neighbor search
class SpatialHashGrid {
public:
    SpatialHashGrid(size_t max_particles, float cell_size = 1.0f);
    ~SpatialHashGrid();
    
    // Build grid from particle positions
    void build(const ParticleData* d_particles);
    
    // Compute forces using spatial hashing (short-range only)
    void computeForces(ParticleData* d_particles, float cutoff, float G, float eps);
    
    // Get grid statistics
    int3 getGridDims() const { return grid_dims_; }
    float getCellSize() const { return cell_size_; }
    int getTotalCells() const { return total_cells_; }
    
    // Access data for testing
    void copyCellDataToHost(std::vector<int>& cell_start, std::vector<int>& cell_end,
                            std::vector<int>& particle_cells, std::vector<int>& sorted_indices);
    
    // Verify cell assignment (for testing)
    bool verifyCellAssignment(const ParticleData* h_particles) const;
    
    // Get cell index for a position
    __host__ __device__ static int3 getCellIndex(float x, float y, float z, float cell_size);
    __host__ __device__ static int hashCell(int3 cell, int3 grid_dims);
    
private:
    // Device memory
    int* d_cell_start_;      // Start index for each cell
    int* d_cell_end_;        // End index for each cell
    int* d_particle_cell_;   // Cell index for each particle
    int* d_sorted_indices_;  // Particle indices sorted by cell
    int* d_cell_counts_;     // Temporary: count per cell
    
    // Grid parameters
    size_t max_particles_;
    float cell_size_;
    int3 grid_dims_;
    int total_cells_;
    
    // Bounding box
    Vec3 bbox_min_;
    Vec3 bbox_max_;
    
    // Internal methods
    void computeBoundingBox(const ParticleData* d_particles);
    void assignParticlesToCells(const ParticleData* d_particles);
    void sortParticlesByCell();
    void computeCellRanges();
};

// GPU kernel declarations
void launchAssignCellsKernel(const ParticleData* d_particles, int* d_particle_cell,
                              float cell_size, const Vec3& bbox_min, int3 grid_dims);
void launchCountCellsKernel(const int* d_particle_cell, int* d_cell_counts,
                             int particle_count, int total_cells);
void launchPrefixSumKernel(int* d_cell_counts, int* d_cell_start, int total_cells);
void launchSortParticlesByCellKernel(const int* d_particle_cell, const int* d_cell_start,
                                      int* d_sorted_indices, int* d_cell_counts_temp,
                                      int particle_count);
void launchComputeCellEndKernel(const int* d_cell_start, int* d_cell_end,
                                 const int* d_cell_counts, int total_cells);
void launchSpatialHashForceKernel(const int* d_cell_start, const int* d_cell_end,
                                   const int* d_sorted_indices,
                                   const ParticleData* d_particles,
                                   float* d_acc_x, float* d_acc_y, float* d_acc_z,
                                   int3 grid_dims, float cell_size, const Vec3& bbox_min,
                                   float cutoff, float G, float eps2, int block_size);

} // namespace nbody
