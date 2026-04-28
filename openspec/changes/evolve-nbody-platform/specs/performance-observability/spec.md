## ADDED Requirements

### Requirement: Benchmark Harnesses
The system SHALL provide dedicated benchmark executables or benchmark modes for the major simulation pipelines so contributors can measure algorithm and subsystem performance independently of the interactive renderer.

#### Scenario: Algorithm benchmarks are available
- **WHEN** a contributor builds the benchmark targets
- **THEN** the repository SHALL provide benchmark coverage for Direct N², Barnes-Hut, Spatial Hash, integration, and relevant data transfer or serialization paths

#### Scenario: Benchmarks run headlessly
- **WHEN** a benchmark is executed in a non-graphical environment
- **THEN** the benchmark SHALL run without requiring an OpenGL window or interactive renderer

### Requirement: Profiling Instrumentation
The system SHALL expose profiling instrumentation for frame phases and major compute stages so developers can attribute time to simulation, data transfer, and rendering work.

#### Scenario: Frame-phase timings are emitted
- **WHEN** profiling instrumentation is enabled
- **THEN** the system SHALL report timings for simulation update, Barnes-Hut tree preparation where applicable, interop buffer updates, and rendering

#### Scenario: Instrumentation remains optional
- **WHEN** profiling support is disabled at build time
- **THEN** the repository SHALL still build and run without requiring profiling tooling at runtime

### Requirement: Structured Performance Reporting
The system SHALL provide machine-readable performance outputs suitable for regression tracking and documentation.

#### Scenario: Benchmark results are exportable
- **WHEN** a benchmark run completes
- **THEN** the system SHALL provide structured results that include the benchmark name, configuration, and measured timing data

#### Scenario: Results preserve configuration context
- **WHEN** performance results are emitted
- **THEN** they SHALL record the force method, particle count, and key tuning parameters used for the measurement
