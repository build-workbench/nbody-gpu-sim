## Why

The repository has drifted away from a single, trustworthy operating model: OpenSpec is present but not consistently treated as the source of truth, documentation is duplicated and uneven, engineering configuration is noisy or weakly pinned, and the public project surfaces do not clearly present the project. This change is needed now to stabilize the repository for a final high-quality closeout cycle and to eliminate maintenance drag before the project moves into low-activity archival mode.

## What Changes

- Normalize the repository around `openspec/` as the only active specification workflow and remove contradictory legacy guidance.
- Introduce a repository governance capability that defines documentation quality, AI instruction files, workflow expectations, Pages positioning, GitHub metadata, hooks, and editor/tooling defaults.
- Rewrite and clarify existing specifications where current requirements are vague, redundant, or not aligned with the implemented system.
- Aggressively prune stale, duplicated, or low-value docs/changelog/site content and keep only project-specific, high-signal documentation.
- Rationalize build, CI, Pages, dependency pinning, LSP, and local automation so the project has a lean and predictable final maintenance baseline.
- Refresh GitHub-facing presentation, including Pages and repository About metadata, so the project is accurately described and easier for new users to evaluate.

## Capabilities

### New Capabilities
- `repository-governance`: Defines the repository structure, OpenSpec-first development workflow, documentation standards, AI instruction files, engineering automation, GitHub presentation, and closeout operating model.

### Modified Capabilities
- `quality-attributes`: Tighten maintainability, documentation accuracy, dependency pinning, validation quality, and archive-readiness requirements for the final stabilization cycle.

## Impact

- Affected specs: `openspec/specs/quality-attributes.md` and a new `repository-governance` capability spec
- Affected documentation: `README*`, `CONTRIBUTING.md`, `AGENTS*`, `QWEN.md`, `docs/`, `changelog/`, `site/`
- Affected engineering/config: `.github/workflows/*`, `.claude/`, `.vscode/`, hooks, Copilot/Claude instruction files, `CMakeLists.txt`, scripts, and dependency/version anchors
- Affected external surfaces: GitHub Pages content and GitHub repository About metadata
