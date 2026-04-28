## ADDED Requirements

### Requirement: Standards-Based Data Export
The system SHALL support exporting simulation state in a standards-aware particle or particle-mesh data format for external analysis and visualization workflows.

#### Scenario: Particle state export
- **WHEN** a user requests standards-based export
- **THEN** the system SHALL write particle positions, velocities, masses, and simulation metadata in an interoperable format

#### Scenario: Export preserves metadata
- **WHEN** data is exported for external tools
- **THEN** the exported dataset SHALL include enough metadata to identify simulation time, force method, and physical parameter settings

### Requirement: Standards-Based Data Import
The system SHALL support importing supported external datasets into the simulation state when the required particle fields and metadata are present.

#### Scenario: Supported import restores simulation state
- **WHEN** a user loads a supported external dataset
- **THEN** the system SHALL reconstruct particle state and simulation parameters required to resume or inspect the simulation

#### Scenario: Unsupported data is rejected explicitly
- **WHEN** an external dataset omits required fields or metadata
- **THEN** the system SHALL fail with a meaningful error describing the missing interoperability requirements

### Requirement: Internal Checkpoints Remain Supported
The system SHALL preserve the existing private checkpoint workflow alongside the new interoperability format.

#### Scenario: Private checkpoint remains available
- **WHEN** a user requests a repository-native checkpoint
- **THEN** the current lightweight project-specific save/load workflow SHALL remain available
