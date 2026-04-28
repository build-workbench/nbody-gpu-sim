## MODIFIED Requirements

### Requirement: Workflow Automation Quality
The repository SHALL keep only meaningful automation, SHALL scope workflow triggers to changes that justify execution, and SHALL validate the supported build, test, and benchmark paths for this platform.

#### Scenario: Workflows validate real repository paths
- **WHEN** CI or Pages workflows execute
- **THEN** they SHALL validate the documented build, test, docs, site, or benchmark pipeline instead of relying only on ceremonial string checks

#### Scenario: Workflow noise is minimized
- **WHEN** repository changes do not affect a workflow's owned surface
- **THEN** that workflow SHALL NOT trigger

#### Scenario: Core validation is automated
- **WHEN** simulation code, build scripts, or validation-related configuration changes
- **THEN** CI SHALL execute the supported core build and test path for the repository
