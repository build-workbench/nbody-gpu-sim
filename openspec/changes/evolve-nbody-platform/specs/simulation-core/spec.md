## MODIFIED Requirements

### Requirement: Barnes-Hut Algorithm
The system SHALL implement Barnes-Hut algorithm for scalable force calculation with a GPU-resident tree build and traversal pipeline.

#### Scenario: Octree Construction
- **WHEN** Barnes-Hut method is selected
- **THEN** the system SHALL construct the octree from particle positions without requiring a CPU-side tree build as the steady-state execution path

#### Scenario: Center of Mass
- **WHEN** tree nodes are built
- **THEN** each node SHALL store center of mass and total mass of contained particles through the same GPU-resident pipeline used for traversal inputs

#### Scenario: Theta Parameter
- **WHEN** traversing the tree for force calculation
- **THEN** the system SHALL use configurable θ parameter for approximation accuracy

#### Scenario: Tree Rebuild
- **WHEN** simulation advances to the next Barnes-Hut step
- **THEN** the system SHALL rebuild the acceleration structure from current particle positions before force traversal

#### Scenario: Algorithm Switching
- **WHEN** user requests different force method
- **THEN** the system SHALL support runtime switching between Direct N², Barnes-Hut, and Spatial Hash

#### Scenario: Scalable execution path
- **WHEN** Barnes-Hut is used for large particle counts
- **THEN** the supported execution path SHALL avoid mandatory device-to-host round-trips for tree construction in normal operation
