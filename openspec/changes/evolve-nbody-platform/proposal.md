## Why

The repository already has a strong CUDA N-body core, but it still behaves more like a research demo than a sustainable simulation platform. The next stage should turn the project into a measurable, debuggable, standards-aware system by closing the biggest gaps identified in the research: missing benchmark/profiling infrastructure, CPU-bound Barnes-Hut tree construction, thin runtime controls, private-only data exchange, and weak automation coverage.

## What Changes

- Add a first-class performance observability surface for benchmarks, profiling instrumentation, and structured performance reporting.
- Extend runtime control with richer CLI parameters and an in-app diagnostics/debug UI for algorithm tuning, statistics, and export actions.
- Upgrade Barnes-Hut from the current hybrid GPU/CPU path toward a GPU-native tree build and frame-pipeline orchestration path, including evaluation of CUDA Graph capture for steady-state simulation loops.
- Add scientific data export/import support beyond the private checkpoint format, centered on interoperable particle-mesh data workflows.
- Strengthen automation so supported build, test, and benchmark paths are validated in CI instead of relying mostly on format and documentation checks.
- Restructure build options so headless/core validation paths can run without requiring the full rendering stack.

## Capabilities

### New Capabilities
- `performance-observability`: Benchmark harnesses, profiling instrumentation, structured metrics capture, and performance reporting workflows for simulation and rendering paths.
- `scientific-data-interoperability`: Standardized simulation data export/import for external analysis, visualization, and reproducible research workflows.

### Modified Capabilities
- `simulation-core`: Barnes-Hut requirements will expand from algorithm availability to a GPU-native build pipeline and scalable execution path.
- `visualization`: Visualization requirements will expand to include runtime diagnostics, tuning controls, and debug overlays/panels.
- `simulation-control`: Runtime control requirements will expand to include richer CLI configuration, benchmark mode, and export-oriented workflows.
- `quality-attributes`: Non-functional requirements will expand to cover benchmarkability, profiling visibility, CI-backed validation, and headless/core build paths.
- `repository-governance`: Automation requirements will expand so CI validates real supported build/test/benchmark paths for this platformization effort.

## Impact

- Affected code: `src/main.cpp`, `src/core/`, `src/cuda/`, `src/render/`, `include/nbody/`, `examples/`, `tests/`, `scripts/`, `.github/workflows/`.
- Affected docs/specs: OpenSpec capabilities above, architecture/performance/setup docs, example guidance, and contributor workflow docs.
- Affected dependencies/systems: benchmark/profiling libraries, scientific data export dependencies, OpenSpec change artifacts, and CI workflow scope.
