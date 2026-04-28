## 1. Build and architecture baseline

- [x] 1.1 Audit current build targets and separate core/headless requirements from render-enabled requirements
- [x] 1.2 Add CMake options and target structure for headless/core validation without mandatory OpenGL linkage
- [x] 1.3 Update build scripts to expose the supported headless/core and render-enabled workflows
- [x] 1.4 Add or update tests that verify the new build-path assumptions and validation entry points

## 2. Performance observability foundation

- [x] 2.1 Add benchmark target scaffolding for major simulation subsystems
- [x] 2.2 Implement benchmark coverage for Direct N², Barnes-Hut, Spatial Hash, integration, and serialization or transfer paths
- [x] 2.3 Add optional profiling instrumentation hooks for frame phases and major compute stages
- [x] 2.4 Add structured performance result output for machine-readable regression tracking
- [x] 2.5 Document the supported benchmark and profiling workflows

## 3. Runtime control and diagnostics surface

- [x] 3.1 Expand command-line parsing for simulation, benchmark, and export configuration
- [x] 3.2 Add validation and error reporting for the new CLI control surface
- [x] 3.3 Introduce an in-application diagnostics/debug panel for runtime tuning and inspection
- [x] 3.4 Surface key runtime metrics and supported tuning controls in the diagnostics UI
- [x] 3.5 Add tests and examples covering the expanded runtime control workflows

## 4. GPU-native Barnes-Hut upgrade

- [ ] 4.1 Add benchmark and correctness baselines for the current Barnes-Hut path before replacing internals
- [ ] 4.2 Move Barnes-Hut tree construction to a GPU-resident steady-state path
- [ ] 4.3 Move center-of-mass preparation into the GPU-resident Barnes-Hut pipeline
- [ ] 4.4 Preserve runtime algorithm switching and public force-calculator interfaces while upgrading internals
- [ ] 4.5 Evaluate and integrate CUDA Graph capture where it improves the steady-state Barnes-Hut frame pipeline
- [ ] 4.6 Add regression tests and benchmark comparisons for Barnes-Hut parity and scaling

## 5. Scientific data interoperability

- [x] 5.1 Choose and integrate the minimum viable standards-aware data library stack
- [x] 5.2 Implement standards-based export for particle state and simulation metadata
- [x] 5.3 Implement supported import paths for interoperable datasets with explicit validation failures
- [x] 5.4 Keep the existing private checkpoint workflow intact alongside the new interoperability path
- [ ] 5.5 Expose standards-based export or import through supported CLI and diagnostics workflows
- [x] 5.6 Add round-trip and metadata coverage tests for the interoperability surface

## 6. Automation, documentation, and finish pass

- [x] 6.1 Expand CI to validate supported core build and test workflows
- [x] 6.2 Add a benchmark smoke-validation workflow with scoped triggers
- [ ] 6.3 Keep Pages and non-owned workflows scoped to relevant changes only
- [ ] 6.4 Update architecture, performance, setup, and examples documentation for the new platform capabilities
- [x] 6.5 Update bilingual counterparts required by the touched core onboarding or specification surfaces
- [ ] 6.6 Run the highest-value existing validation commands for the completed change set and close remaining drift
