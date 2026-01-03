#include "nbody/spatial_hash_grid.hpp"
#include "nbody/force_calculator.hpp"
#include "nbody/error_handling.hpp"
#include <cuda_runtime.h>
#include <thrust/sort.h>
#include <thrust/device_ptr.h>
#include <thrust/scan.h>

namespace nbody {

// Cell index calculation
__host__ __device__ int3 SpatialHashGrid::getCellIndex(float x, float y, float z, float cell_size) {
    return make_int3(
        static_cast<int>(floorf(x / cell_size)),
        static_cast<int>(floorf(y / cell_size)),
        static_cast<int>(floorf(z / cell_size))
    );
}

__host__ __device__ int SpatialHashGrid::hashCell(int3 cell, int3 grid_dims) {
    // Wrap negative indices
    int cx = ((cell.x % grid_dims.x) + grid_dims.x) % grid_dims.x;
    int cy = ((cell.y % grid_dims.y) + grid_dims.y) % grid_dims.y;
    int cz = ((cell.z % grid_dims.z) + grid_dims.z) % grid_dims.z;
    return cx + cy * grid_dims.x + cz * grid_dims.x * grid_dims.y;
}

// Assign particles to cells kernel
__global__ void assignCellsKernel(
    const float* pos_x, const float* pos_y, const float* pos_z,
    int* particle_cell,
    float cell_size, float min_x, float min_y, float min_z,
    int3 grid_dims, int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    float x = pos_x[i] - min_x;
    float y = pos_y[i] - min_y;
    float z = pos_z[i] - min_z;
    
    int3 cell = make_int3(
        static_cast<int>(floorf(x / cell_size)),
        static_cast<int>(floorf(y / cell_size)),
        static_cast<int>(floorf(z / cell_size))
    );
    
    // Clamp to grid bounds
    cell.x = max(0, min(cell.x, grid_dims.x - 1));
    cell.y = max(0, min(cell.y, grid_dims.y - 1));
    cell.z = max(0, min(cell.z, grid_dims.z - 1));
    
    particle_cell[i] = cell.x + cell.y * grid_dims.x + cell.z * grid_dims.x * grid_dims.y;
}

// Count particles per cell kernel
__global__ void countCellsKernel(
    const int* particle_cell,
    int* cell_counts,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    atomicAdd(&cell_counts[particle_cell[i]], 1);
}

// Compute cell end indices
__global__ void computeCellEndKernel(
    const int* cell_start,
    const int* cell_counts,
    int* cell_end,
    int total_cells
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= total_cells) return;
    
    cell_end[i] = cell_start[i] + cell_counts[i];
}

// Sort particles by cell (scatter)
__global__ void scatterParticlesKernel(
    const int* particle_cell,
    const int* cell_start,
    int* cell_counts_temp,
    int* sorted_indices,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    int cell = particle_cell[i];
    int offset = atomicAdd(&cell_counts_temp[cell], 1);
    sorted_indices[cell_start[cell] + offset] = i;
}

// Spatial hash force calculation kernel
__global__ void spatialHashForceKernel(
    const int* cell_start, const int* cell_end,
    const int* sorted_indices,
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    float* acc_x, float* acc_y, float* acc_z,
    int3 grid_dims, float cell_size,
    float min_x, float min_y, float min_z,
    float cutoff2, float G, float eps2, int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    float xi = pos_x[i];
    float yi = pos_y[i];
    float zi = pos_z[i];
    
    float ax = 0.0f, ay = 0.0f, az = 0.0f;
    
    // Get cell of this particle
    int3 cell = make_int3(
        static_cast<int>(floorf((xi - min_x) / cell_size)),
        static_cast<int>(floorf((yi - min_y) / cell_size)),
        static_cast<int>(floorf((zi - min_z) / cell_size))
    );
    
    // Iterate over neighboring cells (3x3x3)
    for (int dz = -1; dz <= 1; dz++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int3 neighbor = make_int3(cell.x + dx, cell.y + dy, cell.z + dz);
                
                // Skip out-of-bounds cells
                if (neighbor.x < 0 || neighbor.x >= grid_dims.x ||
                    neighbor.y < 0 || neighbor.y >= grid_dims.y ||
                    neighbor.z < 0 || neighbor.z >= grid_dims.z) {
                    continue;
                }
                
                int cell_idx = neighbor.x + neighbor.y * grid_dims.x + 
                               neighbor.z * grid_dims.x * grid_dims.y;
                
                int start = cell_start[cell_idx];
                int end = cell_end[cell_idx];
                
                for (int k = start; k < end; k++) {
                    int j = sorted_indices[k];
                    if (j == i) continue;
                    
                    float dx_p = pos_x[j] - xi;
                    float dy_p = pos_y[j] - yi;
                    float dz_p = pos_z[j] - zi;
                    
                    float dist2 = dx_p*dx_p + dy_p*dy_p + dz_p*dz_p;
                    
                    // Only compute force if within cutoff
                    if (dist2 < cutoff2) {
                        dist2 += eps2;
                        float inv_dist = rsqrtf(dist2);
                        float inv_dist3 = inv_dist * inv_dist * inv_dist;
                        float f = G * mass[j] * inv_dist3;
                        
                        ax += f * dx_p;
                        ay += f * dy_p;
                        az += f * dz_p;
                    }
                }
            }
        }
    }
    
    acc_x[i] = ax;
    acc_y[i] = ay;
    acc_z[i] = az;
}

// SpatialHashGrid implementation
SpatialHashGrid::SpatialHashGrid(size_t max_particles, float cell_size)
    : max_particles_(max_particles), cell_size_(cell_size), total_cells_(0) {
    // Allocate device memory
    CUDA_CHECK(cudaMalloc(&d_particle_cell_, max_particles * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_sorted_indices_, max_particles * sizeof(int)));
}

SpatialHashGrid::~SpatialHashGrid() {
    cudaFree(d_cell_start_);
    cudaFree(d_cell_end_);
    cudaFree(d_particle_cell_);
    cudaFree(d_sorted_indices_);
    cudaFree(d_cell_counts_);
}

void SpatialHashGrid::computeBoundingBox(const ParticleData* d_particles) {
    int N = static_cast<int>(d_particles->count);
    
    // Copy positions to host for bounding box computation
    std::vector<float> h_pos_x(N), h_pos_y(N), h_pos_z(N);
    CUDA_CHECK(cudaMemcpy(h_pos_x.data(), d_particles->pos_x, N * sizeof(float), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_pos_y.data(), d_particles->pos_y, N * sizeof(float), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_pos_z.data(), d_particles->pos_z, N * sizeof(float), cudaMemcpyDeviceToHost));
    
    bbox_min_ = Vec3(1e30f, 1e30f, 1e30f);
    bbox_max_ = Vec3(-1e30f, -1e30f, -1e30f);
    
    for (int i = 0; i < N; i++) {
        bbox_min_.x = std::min(bbox_min_.x, h_pos_x[i]);
        bbox_min_.y = std::min(bbox_min_.y, h_pos_y[i]);
        bbox_min_.z = std::min(bbox_min_.z, h_pos_z[i]);
        bbox_max_.x = std::max(bbox_max_.x, h_pos_x[i]);
        bbox_max_.y = std::max(bbox_max_.y, h_pos_y[i]);
        bbox_max_.z = std::max(bbox_max_.z, h_pos_z[i]);
    }
    
    // Add small padding
    bbox_min_.x -= 0.001f;
    bbox_min_.y -= 0.001f;
    bbox_min_.z -= 0.001f;
    bbox_max_.x += 0.001f;
    bbox_max_.y += 0.001f;
    bbox_max_.z += 0.001f;
}

void SpatialHashGrid::build(const ParticleData* d_particles) {
    int N = static_cast<int>(d_particles->count);
    int block_size = 256;
    int num_blocks = (N + block_size - 1) / block_size;
    
    // Compute bounding box
    computeBoundingBox(d_particles);
    
    // Compute grid dimensions
    grid_dims_.x = static_cast<int>(ceilf((bbox_max_.x - bbox_min_.x) / cell_size_)) + 1;
    grid_dims_.y = static_cast<int>(ceilf((bbox_max_.y - bbox_min_.y) / cell_size_)) + 1;
    grid_dims_.z = static_cast<int>(ceilf((bbox_max_.z - bbox_min_.z) / cell_size_)) + 1;
    
    int new_total_cells = grid_dims_.x * grid_dims_.y * grid_dims_.z;
    
    // Reallocate if needed
    if (new_total_cells != total_cells_) {
        if (d_cell_start_) cudaFree(d_cell_start_);
        if (d_cell_end_) cudaFree(d_cell_end_);
        if (d_cell_counts_) cudaFree(d_cell_counts_);
        
        total_cells_ = new_total_cells;
        CUDA_CHECK(cudaMalloc(&d_cell_start_, total_cells_ * sizeof(int)));
        CUDA_CHECK(cudaMalloc(&d_cell_end_, total_cells_ * sizeof(int)));
        CUDA_CHECK(cudaMalloc(&d_cell_counts_, total_cells_ * sizeof(int)));
    }
    
    // Reset cell counts
    CUDA_CHECK(cudaMemset(d_cell_counts_, 0, total_cells_ * sizeof(int)));
    
    // Assign particles to cells
    assignCellsKernel<<<num_blocks, block_size>>>(
        d_particles->pos_x, d_particles->pos_y, d_particles->pos_z,
        d_particle_cell_, cell_size_,
        bbox_min_.x, bbox_min_.y, bbox_min_.z,
        grid_dims_, N
    );
    CUDA_CHECK_KERNEL();
    
    // Count particles per cell
    countCellsKernel<<<num_blocks, block_size>>>(d_particle_cell_, d_cell_counts_, N);
    CUDA_CHECK_KERNEL();
    
    // Prefix sum to get cell start indices
    thrust::device_ptr<int> counts_ptr(d_cell_counts_);
    thrust::device_ptr<int> start_ptr(d_cell_start_);
    thrust::exclusive_scan(counts_ptr, counts_ptr + total_cells_, start_ptr);
    
    // Compute cell end indices
    int cell_blocks = (total_cells_ + block_size - 1) / block_size;
    computeCellEndKernel<<<cell_blocks, block_size>>>(
        d_cell_start_, d_cell_counts_, d_cell_end_, total_cells_
    );
    CUDA_CHECK_KERNEL();
    
    // Reset counts for scatter
    CUDA_CHECK(cudaMemset(d_cell_counts_, 0, total_cells_ * sizeof(int)));
    
    // Scatter particles to sorted order
    scatterParticlesKernel<<<num_blocks, block_size>>>(
        d_particle_cell_, d_cell_start_, d_cell_counts_, d_sorted_indices_, N
    );
    CUDA_CHECK_KERNEL();
}

void SpatialHashGrid::computeForces(ParticleData* d_particles, float cutoff, float G, float eps) {
    int N = static_cast<int>(d_particles->count);
    int block_size = 256;
    int num_blocks = (N + block_size - 1) / block_size;
    
    spatialHashForceKernel<<<num_blocks, block_size>>>(
        d_cell_start_, d_cell_end_, d_sorted_indices_,
        d_particles->pos_x, d_particles->pos_y, d_particles->pos_z,
        d_particles->mass,
        d_particles->acc_x, d_particles->acc_y, d_particles->acc_z,
        grid_dims_, cell_size_,
        bbox_min_.x, bbox_min_.y, bbox_min_.z,
        cutoff * cutoff, G, eps * eps, N
    );
    CUDA_CHECK_KERNEL();
}

void SpatialHashGrid::copyCellDataToHost(std::vector<int>& cell_start, std::vector<int>& cell_end,
                                          std::vector<int>& particle_cells, std::vector<int>& sorted_indices) {
    cell_start.resize(total_cells_);
    cell_end.resize(total_cells_);
    particle_cells.resize(max_particles_);
    sorted_indices.resize(max_particles_);
    
    CUDA_CHECK(cudaMemcpy(cell_start.data(), d_cell_start_, total_cells_ * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(cell_end.data(), d_cell_end_, total_cells_ * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(particle_cells.data(), d_particle_cell_, max_particles_ * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(sorted_indices.data(), d_sorted_indices_, max_particles_ * sizeof(int), cudaMemcpyDeviceToHost));
}

bool SpatialHashGrid::verifyCellAssignment(const ParticleData* h_particles) const {
    for (size_t i = 0; i < h_particles->count; i++) {
        int3 expected_cell = getCellIndex(
            h_particles->pos_x[i] - bbox_min_.x,
            h_particles->pos_y[i] - bbox_min_.y,
            h_particles->pos_z[i] - bbox_min_.z,
            cell_size_
        );
        
        // Clamp to grid bounds
        expected_cell.x = std::max(0, std::min(expected_cell.x, grid_dims_.x - 1));
        expected_cell.y = std::max(0, std::min(expected_cell.y, grid_dims_.y - 1));
        expected_cell.z = std::max(0, std::min(expected_cell.z, grid_dims_.z - 1));
        
        // Verify particle is within cell bounds
        float cell_min_x = bbox_min_.x + expected_cell.x * cell_size_;
        float cell_max_x = cell_min_x + cell_size_;
        float cell_min_y = bbox_min_.y + expected_cell.y * cell_size_;
        float cell_max_y = cell_min_y + cell_size_;
        float cell_min_z = bbox_min_.z + expected_cell.z * cell_size_;
        float cell_max_z = cell_min_z + cell_size_;
        
        if (h_particles->pos_x[i] < cell_min_x || h_particles->pos_x[i] > cell_max_x ||
            h_particles->pos_y[i] < cell_min_y || h_particles->pos_y[i] > cell_max_y ||
            h_particles->pos_z[i] < cell_min_z || h_particles->pos_z[i] > cell_max_z) {
            return false;
        }
    }
    return true;
}

// SpatialHashCalculator implementation
SpatialHashCalculator::SpatialHashCalculator(float cell_size, float cutoff_radius)
    : cell_size_(cell_size), cutoff_radius_(cutoff_radius) {}

SpatialHashCalculator::~SpatialHashCalculator() = default;

void SpatialHashCalculator::computeForces(ParticleData* d_particles) {
    if (!grid_) {
        grid_ = std::make_unique<SpatialHashGrid>(d_particles->count, cell_size_);
    }
    grid_->build(d_particles);
    grid_->computeForces(d_particles, cutoff_radius_, G_, softening_eps_);
}

// Factory function
std::unique_ptr<ForceCalculator> createForceCalculator(ForceMethod method,
                                                        const SimulationConfig& config) {
    switch (method) {
        case ForceMethod::DIRECT_N2:
            return std::make_unique<DirectForceCalculator>(config.cuda_block_size);
        case ForceMethod::BARNES_HUT: {
            auto calc = std::make_unique<BarnesHutCalculator>(config.barnes_hut_theta);
            calc->setGravitationalConstant(config.G);
            calc->setSofteningParameter(config.softening);
            return calc;
        }
        case ForceMethod::SPATIAL_HASH: {
            auto calc = std::make_unique<SpatialHashCalculator>(
                config.spatial_hash_cell_size, config.spatial_hash_cutoff);
            calc->setGravitationalConstant(config.G);
            calc->setSofteningParameter(config.softening);
            return calc;
        }
        default:
            return std::make_unique<DirectForceCalculator>(config.cuda_block_size);
    }
}

} // namespace nbody
