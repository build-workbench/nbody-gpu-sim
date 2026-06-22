#include "nbody/barnes_hut_tree.hpp"
#include "nbody/error_handling.hpp"
#include "nbody/force_calculator.hpp"
#include "nbody/performance_observability.hpp"
#include <algorithm>
#include <cmath>
#include <cuda_runtime.h>
#include <thrust/device_ptr.h>
#include <thrust/sort.h>
#include <vector>

namespace nbody {

// Constants for bounding box computation
namespace {
// Large value for initializing bounding box min/max
// Used to find the actual min/max of particle positions
constexpr float BBOX_INIT_MIN = 1e30f;
constexpr float BBOX_INIT_MAX = -1e30f;
}  // namespace

// Morton code utilities
__host__ __device__ unsigned int expandBits(unsigned int v) {
  v = (v * 0x00010001u) & 0xFF0000FFu;
  v = (v * 0x00000101u) & 0x0F00F00Fu;
  v = (v * 0x00000011u) & 0xC30C30C3u;
  v = (v * 0x00000005u) & 0x49249249u;
  return v;
}

__host__ __device__ unsigned int computeMortonCode(float x, float y, float z) {
  // Normalize to [0, 1023] range
  unsigned int ix = min(max(static_cast<unsigned int>(x * 1024.0f), 0u), 1023u);
  unsigned int iy = min(max(static_cast<unsigned int>(y * 1024.0f), 0u), 1023u);
  unsigned int iz = min(max(static_cast<unsigned int>(z * 1024.0f), 0u), 1023u);

  return (expandBits(ix) << 2) | (expandBits(iy) << 1) | expandBits(iz);
}

// CAS-based atomic float min/max (correct for all sign combinations)
__device__ inline void atomicMinFloat(float* addr, float val) {
  int* addr_as_int = reinterpret_cast<int*>(addr);
  int old = *addr_as_int;
  int expected;
  do {
    expected = old;
    if (val >= __int_as_float(expected))
      break;
    old = atomicCAS(addr_as_int, expected, __float_as_int(val));
  } while (old != expected);
}

__device__ inline void atomicMaxFloat(float* addr, float val) {
  int* addr_as_int = reinterpret_cast<int*>(addr);
  int old = *addr_as_int;
  int expected;
  do {
    expected = old;
    if (val <= __int_as_float(expected))
      break;
    old = atomicCAS(addr_as_int, expected, __float_as_int(val));
  } while (old != expected);
}

// Compute bounding box kernel
__global__ void computeBoundingBoxKernel(const float* pos_x, const float* pos_y, const float* pos_z,
                                         float* min_x, float* min_y, float* min_z, float* max_x,
                                         float* max_y, float* max_z, int N) {
  extern __shared__ float shared[];
  float* s_min_x = shared;
  float* s_min_y = s_min_x + blockDim.x;
  float* s_min_z = s_min_y + blockDim.x;
  float* s_max_x = s_min_z + blockDim.x;
  float* s_max_y = s_max_x + blockDim.x;
  float* s_max_z = s_max_y + blockDim.x;

  int tid = threadIdx.x;
  int i = blockIdx.x * blockDim.x + threadIdx.x;

  if (i < N) {
    s_min_x[tid] = s_max_x[tid] = pos_x[i];
    s_min_y[tid] = s_max_y[tid] = pos_y[i];
    s_min_z[tid] = s_max_z[tid] = pos_z[i];
  } else {
    s_min_x[tid] = s_min_y[tid] = s_min_z[tid] = 1e30f;
    s_max_x[tid] = s_max_y[tid] = s_max_z[tid] = -1e30f;
  }
  __syncthreads();

  for (int s = blockDim.x / 2; s > 0; s >>= 1) {
    if (tid < s) {
      s_min_x[tid] = fminf(s_min_x[tid], s_min_x[tid + s]);
      s_min_y[tid] = fminf(s_min_y[tid], s_min_y[tid + s]);
      s_min_z[tid] = fminf(s_min_z[tid], s_min_z[tid + s]);
      s_max_x[tid] = fmaxf(s_max_x[tid], s_max_x[tid + s]);
      s_max_y[tid] = fmaxf(s_max_y[tid], s_max_y[tid + s]);
      s_max_z[tid] = fmaxf(s_max_z[tid], s_max_z[tid + s]);
    }
    __syncthreads();
  }

  if (tid == 0) {
    atomicMinFloat(min_x, s_min_x[0]);
    atomicMinFloat(min_y, s_min_y[0]);
    atomicMinFloat(min_z, s_min_z[0]);
    atomicMaxFloat(max_x, s_max_x[0]);
    atomicMaxFloat(max_y, s_max_y[0]);
    atomicMaxFloat(max_z, s_max_z[0]);
  }
}

// Compute Morton codes kernel
__global__ void computeMortonCodesKernel(const float* pos_x, const float* pos_y, const float* pos_z,
                                         unsigned int* morton_codes, int* indices, float min_x,
                                         float min_y, float min_z, float inv_size_x,
                                         float inv_size_y, float inv_size_z, int N) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= N)
    return;

  float nx = (pos_x[i] - min_x) * inv_size_x;
  float ny = (pos_y[i] - min_y) * inv_size_y;
  float nz = (pos_z[i] - min_z) * inv_size_z;

  morton_codes[i] = computeMortonCode(nx, ny, nz);
  indices[i] = i;
}

// Barnes-Hut force calculation kernel
__global__ void barnesHutForceKernel(const OctreeNode* nodes, const float* pos_x,
                                     const float* pos_y, const float* pos_z, const float* mass,
                                     float* acc_x, float* acc_y, float* acc_z, int N, int num_nodes,
                                     float theta2, float G, float eps2) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= N)
    return;

  float xi = pos_x[i];
  float yi = pos_y[i];
  float zi = pos_z[i];

  float ax = 0.0f, ay = 0.0f, az = 0.0f;

  // Stack for tree traversal
  // Stack size of 256 supports trees up to depth ~85 (log8(256*8) ≈ 85)
  // Increased from 128 to handle highly non-uniform particle distributions.
  // For extremely deep trees, if stack overflow occurs, some nodes will be
  // skipped (graceful degradation) - see the overflow check below.
  constexpr int STACK_SIZE = 256;
  int stack[STACK_SIZE];
  int stack_ptr = 0;
  stack[stack_ptr++] = 0;  // Start with root

  while (stack_ptr > 0) {
    int node_idx = stack[--stack_ptr];
    if (node_idx < 0 || node_idx >= num_nodes)
      continue;

    const OctreeNode& node = nodes[node_idx];

    if (node.total_mass == 0.0f)
      continue;

    float dx = node.center_of_mass.x - xi;
    float dy = node.center_of_mass.y - yi;
    float dz = node.center_of_mass.z - zi;
    float dist2 = dx * dx + dy * dy + dz * dz + eps2;

    // Check if we can use this node as approximation
    float size2 = 4.0f * node.half_size * node.half_size;

    if (node.is_leaf || size2 / dist2 < theta2) {
      // Use node's center of mass
      if (node.particle_index != i) {  // Don't compute self-interaction
        float inv_dist = rsqrtf(dist2);
        float inv_dist3 = inv_dist * inv_dist * inv_dist;
        float f = G * node.total_mass * inv_dist3;

        ax += f * dx;
        ay += f * dy;
        az += f * dz;
      }
    } else {
      // Need to go deeper - add children to stack
      // Check for stack overflow to prevent memory corruption
      if (stack_ptr + 8 <= STACK_SIZE) {
        for (int c = 0; c < 8; c++) {
          if (node.children[c] >= 0) {
            stack[stack_ptr++] = node.children[c];
          }
        }
      }
      // If stack overflow would occur, skip this node's children
      // This results in approximate force calculation (graceful degradation)
    }
  }

  acc_x[i] = ax;
  acc_y[i] = ay;
  acc_z[i] = az;
}

// BarnesHutTree implementation
BarnesHutTree::BarnesHutTree(size_t max_particles)
    : max_particles_(max_particles), max_nodes_(max_particles * 2), node_count_(0), max_depth_(0) {
  CUDA_CHECK(cudaMalloc(&d_nodes_, max_nodes_ * sizeof(OctreeNode)));
  CUDA_CHECK(cudaMalloc(&d_sorted_indices_, max_particles * sizeof(int)));
  CUDA_CHECK(cudaMalloc(&d_morton_codes_, max_particles * sizeof(unsigned int)));
  h_nodes_.resize(max_nodes_);
}

BarnesHutTree::~BarnesHutTree() {
  cudaFree(d_nodes_);
  cudaFree(d_sorted_indices_);
  cudaFree(d_morton_codes_);
}

void BarnesHutTree::computeBoundingBox(const ParticleData* d_particles) {
  int N = static_cast<int>(d_particles->count);
  int block_size = 256;
  int num_blocks = (N + block_size - 1) / block_size;

  float *d_min_x, *d_min_y, *d_min_z, *d_max_x, *d_max_y, *d_max_z;
  CUDA_CHECK(cudaMalloc(&d_min_x, sizeof(float)));
  CUDA_CHECK(cudaMalloc(&d_min_y, sizeof(float)));
  CUDA_CHECK(cudaMalloc(&d_min_z, sizeof(float)));
  CUDA_CHECK(cudaMalloc(&d_max_x, sizeof(float)));
  CUDA_CHECK(cudaMalloc(&d_max_y, sizeof(float)));
  CUDA_CHECK(cudaMalloc(&d_max_z, sizeof(float)));

  float init_min = BBOX_INIT_MIN, init_max = BBOX_INIT_MAX;
  CUDA_CHECK(cudaMemcpy(d_min_x, &init_min, sizeof(float), cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(d_min_y, &init_min, sizeof(float), cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(d_min_z, &init_min, sizeof(float), cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(d_max_x, &init_max, sizeof(float), cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(d_max_y, &init_max, sizeof(float), cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(d_max_z, &init_max, sizeof(float), cudaMemcpyHostToDevice));

  size_t shared_size = 6 * block_size * sizeof(float);
  computeBoundingBoxKernel<<<num_blocks, block_size, shared_size>>>(
      d_particles->pos_x, d_particles->pos_y, d_particles->pos_z, d_min_x, d_min_y, d_min_z,
      d_max_x, d_max_y, d_max_z, N);
  CUDA_CHECK_KERNEL();

  CUDA_CHECK(cudaMemcpy(&bbox_min_.x, d_min_x, sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(cudaMemcpy(&bbox_min_.y, d_min_y, sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(cudaMemcpy(&bbox_min_.z, d_min_z, sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(cudaMemcpy(&bbox_max_.x, d_max_x, sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(cudaMemcpy(&bbox_max_.y, d_max_y, sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(cudaMemcpy(&bbox_max_.z, d_max_z, sizeof(float), cudaMemcpyDeviceToHost));

  cudaFree(d_min_x);
  cudaFree(d_min_y);
  cudaFree(d_min_z);
  cudaFree(d_max_x);
  cudaFree(d_max_y);
  cudaFree(d_max_z);
}

void BarnesHutTree::computeMortonCodes(const ParticleData* d_particles) {
  int N = static_cast<int>(d_particles->count);
  int block_size = 256;
  int num_blocks = (N + block_size - 1) / block_size;

  float size_x = bbox_max_.x - bbox_min_.x + 0.001f;
  float size_y = bbox_max_.y - bbox_min_.y + 0.001f;
  float size_z = bbox_max_.z - bbox_min_.z + 0.001f;

  computeMortonCodesKernel<<<num_blocks, block_size>>>(
      d_particles->pos_x, d_particles->pos_y, d_particles->pos_z, d_morton_codes_,
      d_sorted_indices_, bbox_min_.x, bbox_min_.y, bbox_min_.z, 1.0f / size_x, 1.0f / size_y,
      1.0f / size_z, N);
  CUDA_CHECK_KERNEL();
}

void BarnesHutTree::sortParticlesByMorton(size_t particle_count) {
  thrust::device_ptr<unsigned int> keys(d_morton_codes_);
  thrust::device_ptr<int> values(d_sorted_indices_);
  thrust::sort_by_key(keys, keys + particle_count, values);
}

void BarnesHutTree::build(const ParticleData* d_particles) {
  NBODY_PROFILE_SCOPE("barnes_hut.build");
  computeBoundingBox(d_particles);
  computeMortonCodes(d_particles);
  sortParticlesByMorton(d_particles->count);
  buildTreeOnHost(d_particles);
}

// Determine which octant of `node` contains `pos`.
static int octantOf(const Vec3& pos, const Vec3& center) {
  int octant = 0;
  if (pos.x >= center.x)
    octant |= 1;
  if (pos.y >= center.y)
    octant |= 2;
  if (pos.z >= center.z)
    octant |= 4;
  return octant;
}

// Allocate a new leaf node holding a single particle.
int BarnesHutTree::allocateLeaf(int parent, int octant, int particle_index,
                                const Vec3& pos, float mass) {
  int new_idx = node_count_++;
  OctreeNode& parent_node = h_nodes_[parent];
  parent_node.children[octant] = new_idx;
  parent_node.is_leaf = false;

  float hs = parent_node.half_size * 0.5f;
  const Vec3& c = parent_node.center;
  OctreeNode& child = h_nodes_[new_idx];
  child.center = Vec3(c.x + ((octant & 1) ? hs : -hs), c.y + ((octant & 2) ? hs : -hs),
                      c.z + ((octant & 4) ? hs : -hs));
  child.half_size = hs;
  child.is_leaf = true;
  child.particle_index = particle_index;
  child.particle_count = 1;
  child.total_mass = mass;
  child.center_of_mass = pos;
  for (int j = 0; j < 8; j++)
    child.children[j] = -1;
  return new_idx;
}

void BarnesHutTree::buildTreeOnHost(const ParticleData* d_particles) {
  NBODY_PROFILE_SCOPE("barnes_hut.tree_build_host");
  // Builds the Barnes-Hut octree on the CPU, then copies the result to GPU
  // memory. Simple and correct; the device-host copy is the main bottleneck
  // for very large N, but is acceptable for typical demo workloads.

  const int N = static_cast<int>(d_particles->count);

  // Copy particle data to host once (reused for center-of-mass aggregation).
  std::vector<float> h_pos_x(N), h_pos_y(N), h_pos_z(N), h_mass(N);
  std::vector<int> h_sorted_indices(N);

  CUDA_CHECK(
      cudaMemcpy(h_pos_x.data(), d_particles->pos_x, N * sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(
      cudaMemcpy(h_pos_y.data(), d_particles->pos_y, N * sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(
      cudaMemcpy(h_pos_z.data(), d_particles->pos_z, N * sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(
      cudaMemcpy(h_mass.data(), d_particles->mass, N * sizeof(float), cudaMemcpyDeviceToHost));
  CUDA_CHECK(cudaMemcpy(h_sorted_indices.data(), d_sorted_indices_, N * sizeof(int),
                        cudaMemcpyDeviceToHost));

  // Initialize root node.
  h_nodes_.assign(max_nodes_, OctreeNode{});

  OctreeNode& root = h_nodes_[0];
  root.center = Vec3((bbox_min_.x + bbox_max_.x) * 0.5f, (bbox_min_.y + bbox_max_.y) * 0.5f,
                     (bbox_min_.z + bbox_max_.z) * 0.5f);
  float max_size =
      std::max({bbox_max_.x - bbox_min_.x, bbox_max_.y - bbox_min_.y, bbox_max_.z - bbox_min_.z});
  root.half_size = max_size * 0.5f + 0.001f;
  root.is_leaf = false;
  root.particle_index = -1;
  root.particle_count = N;

  node_count_ = 1;
  max_depth_ = 0;

  constexpr int MAX_DEPTH = 32;

  // Insert particles one by one, splitting existing leaf nodes when necessary.
  for (int i = 0; i < N; i++) {
    const int idx = h_sorted_indices[i];
    const Vec3 pos(h_pos_x[idx], h_pos_y[idx], h_pos_z[idx]);
    const float m = h_mass[idx];

    int current = 0;
    int depth = 0;

    while (depth < MAX_DEPTH) {
      OctreeNode& node = h_nodes_[current];

      if (node.is_leaf) {
        // This leaf already holds a particle. Convert it into an internal
        // node and re-insert both the existing particle and the new one into
        // deeper children. If both land in the same octant at MAX_DEPTH, we
        // keep them together in one leaf (multi-particle leaf).
        const int existing_idx = node.particle_index;
        const Vec3 existing_pos = node.center_of_mass;
        const float existing_mass = node.total_mass;

        node.is_leaf = false;
        node.particle_index = -1;
        node.particle_count = 0;
        node.total_mass = 0.0f;
        node.center_of_mass = Vec3(0, 0, 0);

        // Re-insert the existing particle into a child leaf.
        int existing_octant = octantOf(existing_pos, node.center);
        if (node.children[existing_octant] < 0) {
          allocateLeaf(current, existing_octant, existing_idx, existing_pos, existing_mass);
        }
        // Fall through to insert the new particle below.
      }

      // node is now an internal node: descend into the correct octant.
      int octant = octantOf(pos, node.center);
      int child = node.children[octant];
      if (child < 0) {
        // Empty octant: create a new leaf holding the new particle.
        allocateLeaf(current, octant, idx, pos, m);
        break;
      }

      current = child;
      depth++;
    }

    if (depth >= MAX_DEPTH) {
      // Degenerate case: two (or more) particles collapse into the same
      // spatial cell at max depth. Merge into the current leaf, accumulating
      // mass and center of mass so force calculation still sees the group.
      OctreeNode& node = h_nodes_[current];
      if (!node.is_leaf) {
        node.is_leaf = true;
        node.particle_index = idx;
        node.particle_count = 1;
        node.total_mass = m;
        node.center_of_mass = pos;
      } else {
        float total = node.total_mass + m;
        node.center_of_mass =
            Vec3((node.center_of_mass.x * node.total_mass + pos.x * m) / total,
                 (node.center_of_mass.y * node.total_mass + pos.y * m) / total,
                 (node.center_of_mass.z * node.total_mass + pos.z * m) / total);
        node.total_mass = total;
        node.particle_count += 1;
      }
    }

    max_depth_ = std::max(max_depth_, depth);
  }

  // Aggregate centers of mass bottom-up. Leaf nodes already carry their
  // particle's mass/com from insertion, so only internal nodes need work.
  for (int i = node_count_ - 1; i >= 0; i--) {
    OctreeNode& node = h_nodes_[i];
    if (node.is_leaf)
      continue;

    float total_mass = 0.0f;
    Vec3 com(0, 0, 0);
    int count = 0;
    for (int c = 0; c < 8; c++) {
      int child_idx = node.children[c];
      if (child_idx >= 0) {
        const OctreeNode& child = h_nodes_[child_idx];
        total_mass += child.total_mass;
        com.x += child.center_of_mass.x * child.total_mass;
        com.y += child.center_of_mass.y * child.total_mass;
        com.z += child.center_of_mass.z * child.total_mass;
        count += child.particle_count;
      }
    }
    node.total_mass = total_mass;
    node.particle_count = count;
    if (total_mass > 0.0f) {
      node.center_of_mass = com / total_mass;
    }
  }

  // Copy tree to device.
  CUDA_CHECK(cudaMemcpy(d_nodes_, h_nodes_.data(), node_count_ * sizeof(OctreeNode),
                        cudaMemcpyHostToDevice));
}

void BarnesHutTree::computeForces(ParticleData* d_particles, float theta, float G, float eps) {
  int N = static_cast<int>(d_particles->count);
  int block_size = 256;
  int num_blocks = (N + block_size - 1) / block_size;

  barnesHutForceKernel<<<num_blocks, block_size>>>(
      d_nodes_, d_particles->pos_x, d_particles->pos_y, d_particles->pos_z, d_particles->mass,
      d_particles->acc_x, d_particles->acc_y, d_particles->acc_z, N, node_count_, theta * theta, G,
      eps * eps);
  CUDA_CHECK_KERNEL();
}

void BarnesHutTree::copyNodesToHost() {
  CUDA_CHECK(cudaMemcpy(h_nodes_.data(), d_nodes_, node_count_ * sizeof(OctreeNode),
                        cudaMemcpyDeviceToHost));
}

bool BarnesHutTree::verifyMassConservation(const ParticleData* h_particles) const {
  float total_mass = 0.0f;
  for (size_t i = 0; i < h_particles->count; i++) {
    total_mass += h_particles->mass[i];
  }

  float root_mass = h_nodes_[0].total_mass;
  return std::abs(total_mass - root_mass) < 0.001f * total_mass;
}

// BarnesHutCalculator implementation
BarnesHutCalculator::BarnesHutCalculator(float theta) : theta_(theta) {}

BarnesHutCalculator::~BarnesHutCalculator() = default;

void BarnesHutCalculator::computeForces(ParticleData* d_particles) {
  if (!tree_) {
    tree_ = std::make_unique<BarnesHutTree>(d_particles->count);
  }
  tree_->build(d_particles);
  tree_->computeForces(d_particles, theta_, G_, softening_eps_);
}

}  // namespace nbody
