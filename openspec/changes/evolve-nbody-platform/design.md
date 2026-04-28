## Context

The repository already has a clear simulation core with `ParticleSystem`, strategy-based force calculators, CUDA/OpenGL interop, and a non-trivial correctness test suite. The current bottlenecks are architectural rather than foundational: Barnes-Hut still crosses the device/host boundary during tree construction, runtime controls are thin, performance visibility is mostly ad hoc, CI does not validate the real build/test path, and the only persisted data format is the project-specific checkpoint format.

This change intentionally groups the roadmap under one umbrella change, but the implementation will still be phased. The design therefore emphasizes dependency order, optional feature toggles, and separation between core simulation validation and render-enabled workflows.

## Goals / Non-Goals

**Goals:**
- Turn the project into a measurable, debuggable, standards-aware simulation platform without discarding the current CUDA/OpenGL architecture.
- Add first-class performance observability through benchmark targets, profiling instrumentation, and structured metrics/reporting.
- Expand runtime control with richer CLI configuration and an in-app diagnostics/debug surface.
- Move Barnes-Hut toward a GPU-native execution path while preserving the existing force-strategy abstraction.
- Add interoperable scientific data export/import alongside the current private checkpoint format.
- Upgrade build and CI flows so core validation can run headlessly and supported workflows are tested automatically.

**Non-Goals:**
- Rewriting rendering to Vulkan, WebGPU, or another graphics API in this change.
- Introducing new physics algorithms such as FMM or distributed multi-GPU execution in this change.
- Replacing the private `.nbody` checkpoint format; it remains the lightweight internal checkpoint path.
- Full performance-portability refactors such as migrating the codebase to Kokkos in this change.

## Decisions

### 1. Use one umbrella change, but implement in dependency-ordered phases

This OpenSpec change covers the full roadmap because the user explicitly wants a single change. The implementation will still be staged internally as: observability/control baseline, Barnes-Hut upgrade, scientific interoperability, and automation hardening. The alternative was to split the work into multiple independent OpenSpec changes, which would reduce review surface but diverge from the requested scope.

### 2. Introduce dedicated observability surfaces instead of hiding performance work inside existing specs

Performance visibility is a product surface, not only an internal implementation detail. The change therefore adds a new `performance-observability` capability for benchmark harnesses, profiling instrumentation, and structured metrics/reporting, while `quality-attributes` is updated only where global non-functional guarantees change. The alternative was to fold all benchmark and profiling behavior into `quality-attributes`, but that would bury user-visible behavior inside broad NFR language.

### 3. Add runtime diagnostics through a dual control plane: CLI + in-app debug UI

The existing application loop already owns GLFW windowing, input callbacks, and renderer state, which makes an in-app panel the most direct way to expose algorithm tuning, frame metrics, and export actions. A richer CLI remains necessary for headless benchmarking and CI execution. The alternative was CLI-only control, but that would not address interactive tuning or visual diagnostics for a render-first application.

### 4. Keep the simulation core/headless path separable from the render-enabled path

The canonical CMake flow will remain the source of truth, but it must support a headless/core validation path that does not require the full OpenGL stack. This enables CI build/test coverage and benchmark execution in environments where rendering dependencies are unavailable. The alternative was to preserve the current unconditional rendering dependency path, which would continue blocking reliable automation and non-graphical workflows.

### 5. Preserve the existing force-strategy API while replacing Barnes-Hut internals incrementally

`ForceCalculator`, `ParticleSystem`, and the current algorithm-selection model are already good extension boundaries. The change therefore upgrades Barnes-Hut behind the existing strategy interface instead of introducing a new public simulation API. The alternative was a larger API redesign around a new execution graph abstraction, but that would expand scope without solving the immediate scaling bottleneck.

### 6. Add standards-based data interchange alongside the private checkpoint path

The current serializer remains the fastest path for project-internal checkpointing, but research workflows need a stable external format with explicit metadata. The change therefore adds a standardized export/import surface for external analysis while keeping the private checkpoint format for fast round-trips. The alternative was to replace the checkpoint format entirely, which would create migration risk without clear user benefit.

### 7. Make CI validate supported workflows instead of only repository hygiene

The repository already documents supported build and test scripts; the automation should validate those paths directly. CI will therefore expand from formatting/docs checks to supported build/test/benchmark jobs, with path scoping retained to avoid noise. The alternative was to keep CI lightweight and rely on manual validation, but that would leave the expanded platform surface without regression protection.

## Risks / Trade-offs

- **[Large umbrella scope]** → Mitigation: keep tasks phase-ordered, with each phase leaving the repository buildable and testable before moving on.
- **[New dependencies increase maintenance]** → Mitigation: gate optional libraries behind explicit CMake options and document bounded/pinned versions.
- **[GPU-native Barnes-Hut may destabilize correctness or performance]** → Mitigation: add benchmark and equivalence checks before replacing the current path, and keep the existing path available until parity is demonstrated.
- **[Debug UI can leak into core code paths]** → Mitigation: keep UI state in the application/render layer and expose simulation changes through existing public control interfaces.
- **[Scientific data support can sprawl]** → Mitigation: limit the first implementation to the minimum interoperable particle-state workflow rather than full ecosystem coverage.
- **[CI runtime cost can grow too much]** → Mitigation: split jobs into headless/core validation, render-capable validation where supported, and small benchmark smoke tests with scoped triggers.

## Migration Plan

1. Land the observability and headless build foundation so the new work can be measured and validated continuously.
2. Upgrade Barnes-Hut internals behind the existing strategy interface, using benchmark and correctness checks to manage cutover risk.
3. Add standards-based data export/import and expose it through CLI/UI workflows.
4. Expand CI and documentation after the supported workflows are stable enough to codify.

Rollback remains possible at phase boundaries because the design preserves the current public interfaces and keeps new surfaces additive wherever possible.

## Open Questions

- Which scientific data library combination best fits the repository’s maintenance target while still delivering useful interoperability?
- Should profiling instrumentation be compiled in by default for debug builds, or enabled only through an explicit CMake option?
- How should performance baselines be stored: repository fixtures, generated artifacts, or documented threshold expectations only?
