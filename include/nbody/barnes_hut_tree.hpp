#pragma once

#include "nbody/types.hpp"
#include <vector>

namespace nbody {

// Octree node structure
struct OctreeNode {
  Vec3 center;          // Node center
  float half_size;      // Half of the node's side length
  Vec3 center_of_mass;  // Center of mass
  float total_mass;     // Total mass in this node
  int children[8];      // Child node indices (-1 means empty)
  int particle_index;   // Particle index for leaf nodes (-1 for internal nodes)
  bool is_leaf;
  int particle_count;  // Number of particles in subtree

  __host__ __device__ OctreeNode()
      : center(),
        half_size(0),
        center_of_mass(),
        total_mass(0),
        particle_index(-1),
        is_leaf(true),
        particle_count(0) {
    for (int i = 0; i < 8; i++)
      children[i] = -1;
  }
};

// Barnes-Hut tree for O(N log N) force calculation
class BarnesHutTree {
public:
  BarnesHutTree(size_t max_particles);
  ~BarnesHutTree();

  // Build tree from particle positions
  void build(const ParticleData* d_particles);

  // Compute forces using Barnes-Hut algorithm
  void computeForces(ParticleData* d_particles, float theta, float G, float eps);

  // Get tree statistics
  int getNodeCount() const { return node_count_; }
  int getMaxDepth() const { return max_depth_; }

  // Access nodes for testing
  const OctreeNode* getNodes() const { return h_nodes_.data(); }
  void copyNodesToHost();

  // Verify mass conservation (for testing)
  bool verifyMassConservation(const ParticleData* h_particles) const;

private:
  // Device memory
  OctreeNode* d_nodes_;
  int* d_sorted_indices_;
  unsigned int* d_morton_codes_;

  // Host memory for verification
  std::vector<OctreeNode> h_nodes_;

  // Tree parameters
  size_t max_particles_;
  size_t max_nodes_;
  int node_count_;
  int max_depth_;

  // Bounding box
  Vec3 bbox_min_;
  Vec3 bbox_max_;

  // Internal methods
  void computeBoundingBox(const ParticleData* d_particles);
  void computeMortonCodes(const ParticleData* d_particles);
  void sortParticlesByMorton(size_t particle_count);
  void buildTreeOnHost(const ParticleData* d_particles);
  int allocateLeaf(int parent, int octant, int particle_index, const Vec3& pos, float mass);
};

// Morton code utilities
__host__ __device__ unsigned int expandBits(unsigned int v);
__host__ __device__ unsigned int computeMortonCode(float x, float y, float z);

}  // namespace nbody
