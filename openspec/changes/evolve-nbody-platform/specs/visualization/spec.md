## ADDED Requirements

### Requirement: Runtime Diagnostics Panel
The visualization layer SHALL expose an in-application diagnostics surface for simulation tuning and inspection.

#### Scenario: Performance metrics are visible in-app
- **WHEN** the simulation is running with the diagnostics surface enabled
- **THEN** the user SHALL be able to inspect current algorithm choice, particle count, FPS, and key frame timing metrics without relying only on the window title

#### Scenario: Runtime tuning controls are available
- **WHEN** the diagnostics surface is enabled
- **THEN** the user SHALL be able to adjust supported runtime parameters such as force method selection and relevant algorithm tuning controls from the application UI

### Requirement: Debug Visualization Controls
The visualization layer SHALL provide debug-oriented display controls for inspecting simulation state.

#### Scenario: Visualization modes can be toggled
- **WHEN** a user enables debug visualization controls
- **THEN** the application SHALL expose supported render and diagnostics modes such as color mapping changes and related inspection overlays
